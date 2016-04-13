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
using CoordinateContainer = Vc::vector<Coordinate<T>, Vc::Allocator<Coordinate<T>>>;
template<typename T>
using PolarCoordinateContainer = Vc::vector<PolarCoordinate<T>, Vc::Allocator<PolarCoordinate<T>>>;

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

template<typename T>
struct AosLayout {
    using IC = CoordinateContainer<typename T::value_type>;
    using OC = PolarCoordinateContainer<typename T::value_type>;

    IC inputValues;
    OC outputValues;

    AosLayout(size_t containerSize) : inputValues(containerSize), outputValues(containerSize){
        simulateInputAos<typename T::value_type>(inputValues, inputValues.size());
    }

    Coordinate<typename T::value_type> coordinate(size_t index) {
        Coordinate<typename T::value_type> r;

        r.x = inputValues[index].x;
        r.y = inputValues[index].y;

        return r;
    }

    void setPolarCoordinate(size_t index, const PolarCoordinate<typename T::value_type> &coord) {
        outputValues[index].radius = coord.radius;
        outputValues[index].phi    = coord.phi;
    }
};

template<typename T>
struct AosSubscriptAccessImpl : public AosLayout<T> {

    AosSubscriptAccessImpl(size_t containerSize) : AosLayout<T>(containerSize) {
    }

    void setupLoop() {
    }

    Coordinate<T> load(size_t index) {
        Coordinate<T> r;

        for (size_t m = 0; m < T::size(); m++) {
          r.x[m] = AosLayout<T>::inputValues[(index + m)].x;
          r.y[m] = AosLayout<T>::inputValues[(index + m)].y;
        }

        return r;
    }

    void store(size_t index, const PolarCoordinate<T> &coord) {
      for (size_t m = 0; m < T::size(); m++) {
        AosLayout<T>::outputValues[(index + m)].radius = coord.radius[m];
        AosLayout<T>::outputValues[(index + m)].phi = coord.phi[m];
      }
    }
};

template<typename T>
struct InterleavedAccessImpl : public AosLayout<T> {
    using IW = InterleavedMemoryWrapper<Coordinate<typename T::value_type>, T>;
    using OW = InterleavedMemoryWrapper<PolarCoordinate<typename T::value_type>, T>;

    IW inputWrapper;
    OW outputWrapper;

    InterleavedAccessImpl(size_t containerSize)
    : AosLayout<T>(containerSize), inputWrapper(AosLayout<T>::inputValues.data()),
      outputWrapper(AosLayout<T>::outputValues.data()) {
    }

    void setupLoop() {
    }

    Coordinate<T> load(size_t index) {
        Coordinate<T> r;

        Vc::tie(r.x, r.y) = inputWrapper[index];
        return r;
    }

    void store(size_t index, const PolarCoordinate<T> &coord) {
        outputWrapper[index] = Vc::tie(coord.radius, coord.phi);
    }
};

template<typename T>
struct AosGatherScatterAccessImpl : public AosLayout<T> {
typedef typename T::IndexType IT;

    IT indexes;

    AosGatherScatterAccessImpl(size_t containerSize)
        : AosLayout<T>(containerSize),
        indexes(IT::IndexesFromZero()) {
    }

    void setupLoop() {
        indexes = IT::IndexesFromZero();
    }

    Coordinate<T> load(size_t index) {
        Coordinate<T> r;

        r.x = AosLayout<T>::inputValues[indexes][&Coordinate<typename T::value_type>::x];
        r.y = AosLayout<T>::inputValues[indexes][&Coordinate<typename T::value_type>::y];

        return r;
    }

    void store(size_t index, const PolarCoordinate<T> &coord) {
        AosLayout<T>::outputValues[indexes][&PolarCoordinate<typename T::value_type>::radius] = coord.radius;
        AosLayout<T>::outputValues[indexes][&PolarCoordinate<typename T::value_type>::phi] = coord.phi;

        indexes += T::size();
    }
};

struct AosSubscriptAccess {
    template<typename T> using type = AosSubscriptAccessImpl<T>;
};

struct InterleavedAccess {
    template<typename T> using type = InterleavedAccessImpl<T>;
};

struct AosGatherScatterAccess {
    template<typename T> using type = AosGatherScatterAccessImpl<T>;
};

struct RestScalar {};
struct Padding {};

template<typename TT>
inline void veryMelone(benchmark::State &state) {
    using T  = typename TT::template at<0>;
    using A  = typename TT::template at<1>;
    using B  = typename TT::template at<2>;

    using P = typename A::template type<T>;

  const size_t inputSize = state.range_x();
  const size_t missingSize = std::is_same<B, RestScalar>::value ? inputSize % T::size() : 0;
  const size_t containerSize = std::is_same<B, RestScalar>::value ? inputSize - missingSize : numberOfChunks(inputSize, T::size()) * T::size();

  P magic(containerSize + missingSize);

  while (state.KeepRunning()) {
      magic.setupLoop();

    for (size_t n = 0; n < containerSize; n += T::size()) {
      //! Loads the values to vc-vector
      const auto coord = magic.load(n);

      //! Calculate the polarcoordinates
      const auto polarCoord = calculatePolarCoordinate(coord); //Ändern

      //! Store the values from the vc-vector
      magic.store(n, polarCoord);
    }

    for (size_t n = (inputSize - missingSize); n < inputSize; n++) {
      magic.setPolarCoordinate(n,
          calculatePolarCoordinate(magic.coordinate(n))); //Gibt Coordinate FLOAT
    }
  }

  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(typename T::value_type));
}
#endif // AOS_H
