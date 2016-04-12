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
#ifndef SOA_H
#define SOA_H
#include <vector>
#include <Vc/vector>

template <typename T> struct ArrayOfCoordinates {
  T x;
  T y;
};

template <typename T> struct ArrayOfPolarCoordinates {
  T radius;
  T phi;
};

template<typename T>
using StdArrayOfCoordinates = ArrayOfCoordinates<std::vector<T, Vc::Allocator<T>>>;
template<typename T>
using VcArrayOfCoordinates = ArrayOfCoordinates<Vc::vector<T, Vc::Allocator<T>>>;

template<typename T>
using StdArrayOfPolarCoordinates = ArrayOfPolarCoordinates<std::vector<T, Vc::Allocator<T>>>;
template<typename T>
using VcArrayOfPolarCoordinates = ArrayOfPolarCoordinates<Vc::vector<T, Vc::Allocator<T>>>;

//! Creates random numbers for SoA
template <typename B, typename T>
void simulateInputSoa(ArrayOfCoordinates<T> &input, const size_t size) {
  using Dist = typename std::conditional<std::is_integral<B>::value,
  std::uniform_int_distribution<B>, std::uniform_real_distribution<B>>::type;
  std::mt19937 engine(std::random_device{}());
  Dist random(std::numeric_limits<B>::min(), std::numeric_limits<B>::max());

  for (size_t n = 0; n < size; n++) {
    input.x[n] = random(engine);
    input.y[n] = random(engine);
  }
}

//! SoA with a padding
template<typename T>
void soaWithPadding(benchmark::State &state) {
  const size_t inputSize = state.range_x();
  const size_t containerSize =
      (numberOfChunks(inputSize, T::size()) * T::size());

  StdArrayOfCoordinates<typename T::value_type> inputValues;
  StdArrayOfPolarCoordinates<typename T::value_type> outputValues;

  T vCoordinateX;
  T vCoordinateY;
  T vRadius;
  T vPhi;

  inputValues.x.reserve(containerSize);
  inputValues.y.reserve(containerSize);

  outputValues.radius.reserve(containerSize);
  outputValues.phi.reserve(containerSize);

  //! The values of the last container are set to 1.0f for the padding
  for (size_t n = 1; n <= T::size(); n++) {
    inputValues.x[(containerSize - n)] = 1.0f;
    inputValues.y[(containerSize - n)] = 1.0f;
  }

  simulateInputSoa<typename T::value_type>(inputValues, inputSize);

  while (state.KeepRunning()) {
    for (size_t n = 0; n < containerSize; n += T::size()) {
      for (size_t m = 0; m < T::size(); m++) {
        vCoordinateX[m] = inputValues.x[(n + m)];
        vCoordinateY[m] = inputValues.y[(n + m)];
      }

      std::tie(vRadius, vPhi) = calculatePolarCoordinate(vCoordinateX, vCoordinateY);

      for (size_t m = 0; m < T::size(); m++) {
        outputValues.radius[(n + m)] = vRadius[m];
        outputValues.phi[(n + m)] = vPhi[m];
      }
    }
  }

  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(typename T::value_type));

#ifdef USE_LOG
  std::clog << "Finnished: SoA_Padding\n";
#endif
}

//! SoA using load and store, with a padding
template<typename T>
void soaWithLoadStorePadding(benchmark::State &state) {
  const size_t inputSize = state.range_x();
  const size_t containerSize =
      (numberOfChunks(inputSize, T::size()) * T::size());

  StdArrayOfCoordinates<typename T::value_type> inputValues;
  StdArrayOfPolarCoordinates<typename T::value_type> outputValues;

  T vCoordinateX;
  T vCoordinateY;
  T vRadius;
  T vPhi;

  inputValues.x.reserve(containerSize);
  inputValues.y.reserve(containerSize);

  outputValues.radius.reserve(containerSize);
  outputValues.phi.reserve(containerSize);

  //! The values of the last container are set to 1.0f for the padding
  for (size_t n = 1; n <= T::size(); n++) {
    inputValues.x[(containerSize - n)] = 1.0f;
    inputValues.y[(containerSize - n)] = 1.0f;
  }

  simulateInputSoa<typename T::value_type>(inputValues, inputSize);

  while (state.KeepRunning()) {
    for (size_t n = 0; n < containerSize; n += T::size()) {
      vCoordinateX.load((inputValues.x.data() + n));
      vCoordinateY.load((inputValues.y.data() + n));

      std::tie(vRadius, vPhi) = calculatePolarCoordinate(vCoordinateX, vCoordinateY);

      vRadius.store((outputValues.radius.data() + n));
      vPhi.store((outputValues.phi.data() + n));
    }
  }

  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(typename T::value_type));

#ifdef USE_LOG
  std::clog << "Finnished: SoA_LoadStore_Padding\n";
#endif
}

//! SoA using gather and scatter as operator, with a padding
template<typename T>
void soaWithGatherScatterPadding(benchmark::State &state) {
typedef typename T::IndexType IT;

  const size_t inputSize = state.range_x();
  const size_t containerSize =
      (numberOfChunks(inputSize, T::size()) * T::size());

  VcArrayOfCoordinates<typename T::value_type> inputValues;
  VcArrayOfPolarCoordinates<typename T::value_type> outputValues;

  T vCoordinateX;
  T vCoordinateY;
  T vRadius;
  T vPhi;

  IT indexes(IT::IndexesFromZero());

  inputValues.x.reserve(containerSize);
  inputValues.y.reserve(containerSize);

  outputValues.radius.reserve(containerSize);
  outputValues.phi.reserve(containerSize);

  //! The values of the last container are set to 1.0f for the padding
  for (size_t n = 1; n <= T::size(); n++) {
    inputValues.x[(containerSize - n)] = 1.0f;
    inputValues.y[(containerSize - n)] = 1.0f;
  }

  simulateInputSoa<typename T::value_type>(inputValues, inputSize);

  while (state.KeepRunning()) {
    indexes = IT::IndexesFromZero();
    for (size_t n = 0; n < containerSize; n += T::size()) {
      vCoordinateX = inputValues.x[indexes];
      vCoordinateY = inputValues.y[indexes];

      std::tie(vRadius, vPhi) = calculatePolarCoordinate(vCoordinateX, vCoordinateY);

      outputValues.radius[indexes] = vRadius;
      outputValues.phi[indexes] = vPhi;
      indexes += T::size();
    }
  }

  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(typename T::value_type));

#ifdef USE_LOG
  std::clog << "Finnished: SoA_GatherScatter[]_Padding\n";
#endif
}

//! SoA using gather and scatter as function, with a padding
template<typename T>
void soaWithGatherScatterAsFunctionPadding(benchmark::State &state) {
typedef typename T::IndexType IT;

  const size_t inputSize = state.range_x();
  const size_t containerSize =
      (numberOfChunks(inputSize, T::size()) * T::size());

  StdArrayOfCoordinates<typename T::value_type> inputValues;
  StdArrayOfPolarCoordinates<typename T::value_type> outputValues;

  T vCoordinateX;
  T vCoordinateY;
  T vRadius;
  T vPhi;

  IT indexes(IT::IndexesFromZero());

  inputValues.x.reserve(containerSize);
  inputValues.y.reserve(containerSize);

  outputValues.radius.reserve(containerSize);
  outputValues.phi.reserve(containerSize);

  //! The values of the last container are set to 1.0f for the padding
  for (size_t n = 1; n <= T::size(); n++) {
    inputValues.x[(containerSize - n)] = 1.0f;
    inputValues.y[(containerSize - n)] = 1.0f;
  }

  simulateInputSoa<typename T::value_type>(inputValues, inputSize);

  while (state.KeepRunning()) {
    indexes = IT::IndexesFromZero();
    for (size_t n = 0; n < containerSize; n += T::size()) {
      vCoordinateX.gather(inputValues.x.data(), indexes);
      vCoordinateY.gather(inputValues.y.data(), indexes);

      std::tie(vRadius, vPhi) = calculatePolarCoordinate(vCoordinateX, vCoordinateY);

      vRadius.scatter(outputValues.radius.data(), indexes);
      vPhi.scatter(outputValues.phi.data(), indexes);
      indexes += T::size();
    }
  }

  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(typename T::value_type));

#ifdef USE_LOG
  std::clog << "Finnished: SoA_GatherScatter()_Padding\n";
#endif
}

//! SoA with rest scalar
template<typename T>
void soaWithRestScalar(benchmark::State &state) {
  const size_t inputSize = state.range_x();
  const size_t missingSize = (inputSize % T::size());

  StdArrayOfCoordinates<typename T::value_type> inputValues;
  StdArrayOfPolarCoordinates<typename T::value_type> outputValues;

  T vCoordinateX;
  T vCoordinateY;
  T vRadius;
  T vPhi;

  inputValues.x.reserve(inputSize);
  inputValues.y.reserve(inputSize);

  outputValues.radius.reserve(inputSize);
  outputValues.phi.reserve(inputSize);

  simulateInputSoa<typename T::value_type>(inputValues, inputSize);

  while (state.KeepRunning()) {
    for (size_t n = 0; n < (inputSize - missingSize); n += T::size()) {
      for (size_t m = 0; m < T::size(); m++) {
        vCoordinateX[m] = inputValues.x[(n + m)];
        vCoordinateY[m] = inputValues.y[(n + m)];
      }

      std::tie(vRadius, vPhi) = calculatePolarCoordinate(vCoordinateX, vCoordinateY);

      for (size_t m = 0; m < T::size(); m++) {
        outputValues.radius[(n + m)] = vRadius[m];
        outputValues.phi[(n + m)] = vPhi[m];
      }
    }

    for (size_t n = (inputSize - missingSize); n < inputSize; n++) {
      std::tie(outputValues.radius[n], outputValues.phi[n]) =
          calculatePolarCoordinate(inputValues.x[n], inputValues.y[n]);
    }
  }

  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(typename T::value_type));

#ifdef USE_LOG
  std::clog << "Finnished: SoA_RestScalar\n";
#endif
}

//! SoA using load and Store, with rest scalar
template<typename T>
void soaWithLoadStoreRestScalar(benchmark::State &state) {
  const size_t inputSize = state.range_x();
  const size_t missingSize = (inputSize % T::size());

  StdArrayOfCoordinates<typename T::value_type> inputValues;
  StdArrayOfPolarCoordinates<typename T::value_type> outputValues;

  T vCoordinateX;
  T vCoordinateY;
  T vRadius;
  T vPhi;

  inputValues.x.reserve(inputSize);
  inputValues.y.reserve(inputSize);

  outputValues.radius.reserve(inputSize);
  outputValues.phi.reserve(inputSize);

  simulateInputSoa<typename T::value_type>(inputValues, inputSize);

  while (state.KeepRunning()) {
    for (size_t n = 0; n < (inputSize - missingSize); n += T::size()) {
      vCoordinateX.load((inputValues.x.data() + n));
      vCoordinateY.load((inputValues.y.data() + n));

      std::tie(vRadius, vPhi) = calculatePolarCoordinate(vCoordinateX, vCoordinateY);

      vRadius.store((outputValues.radius.data() + n));
      vPhi.store((outputValues.phi.data() + n));
    }

    for (size_t n = (inputSize - missingSize); n < inputSize; n++) {
      std::tie(outputValues.radius[n], outputValues.phi[n]) =
          calculatePolarCoordinate(inputValues.x[n], inputValues.y[n]);
    }
  }

  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(typename T::value_type));

#ifdef USE_LOG
  std::clog << "Finnished: SoA_LoadStore_RestScalar\n";
#endif
}

//! SoA using gather and scatter as operator, with rest scalar
template<typename T>
void soaWithGatherScatterRestScalar(benchmark::State &state) {
typedef typename T::IndexType IT;

  const size_t inputSize = state.range_x();
  const size_t missingSize = (inputSize % T::size());

  StdArrayOfCoordinates<typename T::value_type> inputValues;
  StdArrayOfPolarCoordinates<typename T::value_type> outputValues;

  T vCoordinateX;
  T vCoordinateY;
  T vRadius;
  T vPhi;

  IT indexes(IT::IndexesFromZero());

  inputValues.x.reserve(inputSize);
  inputValues.y.reserve(inputSize);

  outputValues.radius.reserve(inputSize);
  outputValues.phi.reserve(inputSize);

  simulateInputSoa<typename T::value_type>(inputValues, inputSize);

  while (state.KeepRunning()) {
    indexes = IT::IndexesFromZero();
    for (size_t n = 0; n < (inputSize - missingSize); n += T::size()) {
      vCoordinateX.gather(inputValues.x.data(), indexes);
      vCoordinateY.gather(inputValues.y.data(), indexes);

      std::tie(vRadius, vPhi) = calculatePolarCoordinate(vCoordinateX, vCoordinateY);

      vRadius.scatter(outputValues.radius.data(), indexes);
      vPhi.scatter(outputValues.phi.data(), indexes);
      indexes += T::size();
    }

    for (size_t n = (inputSize - missingSize); n < inputSize; n++) {
      std::tie(outputValues.radius[n], outputValues.phi[n]) =
          calculatePolarCoordinate(inputValues.x[n], inputValues.y[n]);
    }
  }

  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(typename T::value_type));

#ifdef USE_LOG
  std::clog << "Finnished: SoA_GatherScatter[]_RestScalar\n";
#endif
}

//! SoA using gather and scatter as function, with rest scalar
template<typename T>
void soaWithGatherScatterFunctionRestScalar(benchmark::State &state) {
typedef typename T::IndexType IT;

  const size_t inputSize = state.range_x();
  const size_t missingSize = (inputSize % T::size());

  VcArrayOfCoordinates<typename T::value_type> inputValues;
  VcArrayOfPolarCoordinates<typename T::value_type> outputValues;

  T vCoordinateX;
  T vCoordinateY;
  T vRadius;
  T vPhi;
  IT indexes(IT::IndexesFromZero());

  inputValues.x.reserve(inputSize);
  inputValues.y.reserve(inputSize);

  outputValues.radius.reserve(inputSize);
  outputValues.phi.reserve(inputSize);

  simulateInputSoa<typename T::value_type>(inputValues, inputSize);

  while (state.KeepRunning()) {
    indexes = IT::IndexesFromZero();
    for (size_t n = 0; n < (inputSize - missingSize); n += T::size()) {
      vCoordinateX.gather(inputValues.x.data(), indexes);
      vCoordinateY.gather(inputValues.y.data(), indexes);

      std::tie(vRadius, vPhi) = calculatePolarCoordinate(vCoordinateX, vCoordinateY);

      vRadius.scatter(outputValues.radius.data(), indexes);
      vPhi.scatter(outputValues.phi.data(), indexes);
      indexes += T::size();
    }

    for (size_t n = (inputSize - missingSize); n < inputSize; n++) {
      std::tie(outputValues.radius[n], outputValues.phi[n]) =
          calculatePolarCoordinate(inputValues.x[n], inputValues.y[n]);
    }
  }

  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(typename T::value_type));

#ifdef USE_LOG
  std::clog << "Finnished: SoA_GatherScatter()_RestScalar\n";
#endif
}
#endif // SOA_H
