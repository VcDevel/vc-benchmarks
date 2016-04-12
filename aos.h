/*Copyright © 2016 Björn Gaier

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the names of contributing organizations nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/
#ifndef AOS_H
#define AOS_H
#include <string>
#include <vector>
#include <Vc/vector>

using Vc::Common::InterleavedMemoryWrapper;

template<typename T>
struct Coordinate {
  T x;
  T y;
};

template<typename T>
struct PolarCoordinate {
  T radius;
  T phi;
};

template<typename T>
using VectorCoordinate = Vc::vector<Coordinate<T>, Vc::Allocator<Coordinate<T>>>;
template<typename T>
using VectorPolarCoordinate = Vc::vector<PolarCoordinate<T>, Vc::Allocator<PolarCoordinate<T>>>;

//! Creates random numbers for AoS
template <typename B, typename T>
void simulateInputAos(T &input, const size_t size) {
  typedef typename T::iterator Iterator;
  using Dist = typename std::conditional<std::is_integral<B>::value,
  std::uniform_int_distribution<B>, std::uniform_real_distribution<B>>::type;

  std::mt19937 engine(std::random_device{}());
  Dist random(std::numeric_limits<B>::min(), std::numeric_limits<B>::max());
  Iterator aktElement(input.begin());
  Iterator endElement(input.begin() + size);

  while (aktElement != endElement) {
    aktElement->x = random(engine);
    aktElement->y = random(engine);
    aktElement++;
  }
}

template<typename T, bool B>
struct PaddingPolicy {
    static constexpr bool useRestScalar = B;

    static VectorCoordinate<typename T::value_type>& constructAccessRead(VectorCoordinate<typename T::value_type> &data) {
        return data;
    }

    static VectorPolarCoordinate<typename T::value_type>& constructAccessWrite(VectorPolarCoordinate<typename T::value_type> &data) {
        return data;
    }

    static std::string getLogName() {
        std::string log("AoS with ");
        if(useRestScalar) {
            return log.append("rest scalar");
        }

        else {
            return log.append("padding");
        }
    }
};

template<typename T, bool B>
struct InterleavedPolicy {
    static constexpr bool useRestScalar = B;

    static InterleavedMemoryWrapper<Coordinate<typename T::value_type>, T> constructAccessRead(VectorCoordinate<typename T::value_type> &data) {
        return InterleavedMemoryWrapper<Coordinate<typename T::value_type>, T>(data.data());
    }

    static InterleavedMemoryWrapper<PolarCoordinate<typename T::value_type>, T> constructAccessWrite(VectorPolarCoordinate<typename T::value_type> &data) {
        return InterleavedMemoryWrapper<PolarCoordinate<typename T::value_type>, T>(data.data());
    }

    static std::string getLogName() {
        std::string log("AoS with interleaved memory and ");
        if(useRestScalar) {
            return std::string("rest scalar");
        }

        else {
            return std::string("padding");
        }
    }
};

template<typename T, bool B>
struct GatherScatterPolicy : PaddingPolicy<T, B> {
    static std::string getLogName() {
        std::string log("AoS with gather/scatter and ");
        if(PaddingPolicy<T, B>::useRestScalar) {
            return log.append("rest scalar");
        }

        else {
            return log.append("padding");
        }
    }
};

template<typename T>
using PaddingPolicyPadding = PaddingPolicy<T, false>;
template<typename T>
using InterleavedPolicyPadding = InterleavedPolicy<T, false>;
template<typename T>
using GatherScatterPolicyPadding = GatherScatterPolicy<T, false>;

template<typename T>
using PaddingPolicyRestScalar = PaddingPolicy<T, true>;
template<typename T>
using InterleavedPolicyRestScalar = InterleavedPolicy<T, true>;
template<typename T>
using GatherScatterPolicyRestScalar = GatherScatterPolicy<T, true>;

template<typename T, template<typename> class P, typename U, typename S, typename R>
inline void aosWith(benchmark::State &state, U loopSetup,  S load, R store) {
  const size_t inputSize = state.range_x();
  const size_t missingSize = P<T>::useRestScalar ? inputSize % T::size() : 0;
  const size_t containerSize = (P<T>::useRestScalar) ? inputSize - missingSize : numberOfChunks(inputSize, T::size()) * T::size();

  VectorCoordinate<typename T::value_type>  inputValues(containerSize + missingSize);
  VectorPolarCoordinate<typename T::value_type> outputValues(containerSize + missingSize);

  simulateInputAos<typename T::value_type>(inputValues, inputValues.size());

  const auto &inputAccess = P<T>::constructAccessRead(inputValues);
  auto &&outputAccess = P<T>::constructAccessWrite(outputValues);

  T vCoordinateX;
  T vCoordinateY;

  T vRadius;
  T vPhi;

  while (state.KeepRunning()) {
      loopSetup();

    for (size_t n = 0; n < containerSize; n += T::size()) {
      //! Loads the values to vc-vector
      load(n, vCoordinateX, vCoordinateY, inputAccess);

      //! Calculate the polarcoordinates
      std::tie(vRadius, vPhi) = calculatePolarCoordinate(vCoordinateX, vCoordinateY);

      //! Store the values from the vc-vector
      store(n, vRadius, vPhi, outputAccess);
    }

    for (size_t n = (inputSize - missingSize); n < inputSize; n++) {
      std::tie(outputValues[n].radius, outputValues[n].phi) =
          calculatePolarCoordinate(inputValues[n].x, inputValues[n].y);
    }
  }

  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(typename T::value_type));
}

template<typename T, template <typename> class P>
void aosNormalWith(benchmark::State &state) {
    aosWith<T, P>(state, []{}, [](size_t n, T &vCoordinateX, T &vCoordinateY, const VectorCoordinate<typename T::value_type> &inputValues) {
      for (size_t m = 0; m < T::size(); m++) {
        vCoordinateX[m] = inputValues[(n + m)].x;
        vCoordinateY[m] = inputValues[(n + m)].y;
      }
    }, [](size_t n, T &vRadius, T &vPhi, VectorPolarCoordinate<typename T::value_type> &outputValues) {
      for (size_t m = 0; m < T::size(); m++) {
        outputValues[(n + m)].radius = vRadius[m];
        outputValues[(n + m)].phi = vPhi[m];
      }
    });

#ifdef USE_LOG
  std::clog << "Finnished: " << P<T>::getLogName() << "\n";
#endif
}

template <typename T, template<typename> class P>
void aosInterleavedWith(benchmark::State &state) {

    aosWith<T, P>(state, [](){}, [](size_t n, T &vCoordinateX, T &vCoordinateY, const InterleavedMemoryWrapper<Coordinate<typename T::value_type>, T> &wrapperInput){
        Vc::tie(vCoordinateX, vCoordinateY) = wrapperInput[n];
    }, [](size_t n, T &vRadius, T &vPhi, InterleavedMemoryWrapper<PolarCoordinate<typename T::value_type>, T> &wrapperOutput){
        wrapperOutput[n] = Vc::tie(vRadius, vPhi);
    });

#ifdef USE_LOG
  std::clog << "Finnished: " << P<T>::getLogName() << "\n";
#endif
}

template<typename T, template<typename> class P>
void aosGatherScatterWith(benchmark::State &state) {
typedef typename T::IndexType IT;

IT indexes(IT::IndexesFromZero());

  aosWith<T, P>(state, [&indexes](){
    indexes = IT::IndexesFromZero();
  }, [&indexes](size_t n, T &vCoordinateX, T &vCoordinateY, const VectorCoordinate<typename T::value_type> &inputValues){
      vCoordinateX = inputValues[indexes][&Coordinate<typename T::value_type>::x];
      vCoordinateY = inputValues[indexes][&Coordinate<typename T::value_type>::y];
  }, [&indexes](size_t n, T &vRadius, T &vPhi, VectorPolarCoordinate<typename T::value_type> &outputValues){
      outputValues[indexes][&PolarCoordinate<typename T::value_type>::radius] = vRadius;
      outputValues[indexes][&PolarCoordinate<typename T::value_type>::phi] = vPhi;

      indexes += T::size();
  });

#ifdef USE_LOG
  std::clog << "Finnished: " << P<T>::getLogName() << "\n";
#endif
}

//! AoS with a padding
template<typename T>
void aosWithPadding(benchmark::State &state) {

    aosNormalWith<T, PaddingPolicyPadding>(state);
}

//! AoS unsing interleaved memory with a padding--------------------------------------------------------------------------------------------------------------------------------------------
template <typename T>
void aosWithInterleavedPadding(benchmark::State &state) {

    aosInterleavedWith<T, InterleavedPolicyPadding>(state);
}

template<typename T>
//! Aos using Gather and Scatter as operator, with a padding--------------------------------------------------------------------------------------------
void aosWithGatherScatterPadding(benchmark::State &state) {

  aosGatherScatterWith<T, GatherScatterPolicyPadding>(state);
}

//! AoS with rest scalar
template<typename T>
void aosWithRestScalar(benchmark::State &state) {

   aosNormalWith<T, PaddingPolicyRestScalar>(state);
}

template<typename T>
//! AoS using interleaved memory, with rest scalar
void aosWithInterleavedRestScalar(benchmark::State &state) {

  aosInterleavedWith<T, InterleavedPolicyRestScalar>(state);
}

//! AoS using gather and scatter as operator, with rest scalar
template <typename T>
void aosWithGatherScatterRestScalar(benchmark::State &state) {

   aosGatherScatterWith<T, GatherScatterPolicyRestScalar>(state);
}
#endif // AOS_H
