/*Copyright © 2016 Björn Gaier
All rights reserved.

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

//! The type of the indices for gather and scatter
//typedef float_v::IndexType IT;

//! Holds a point in 2D space
template<typename T>
struct Coordinate {
  T x;
  T y;
};

//! Holds a point as a polarcoordinate. Saves the results from calculation
template<typename T>
struct PolarCoordinate {
  T radius;
  T phi;
};

template<typename T>
using VectorCoordinate = std::vector<Coordinate<T>, Vc::Allocator<Coordinate<T>>>;
template<typename T>
using VectorPolarCoordinate = std::vector<PolarCoordinate<T>, Vc::Allocator<PolarCoordinate<T>>>;

template<typename T>
using VcVectorCoordinate = Vc::vector<Coordinate<T>, Vc::Allocator<Coordinate<T>>>;
template<typename T>
using VcVectorPolarCoordinate = Vc::vector<PolarCoordinate<T>, Vc::Allocator<PolarCoordinate<T>>>;

//! Creates random numbers for AoS
template <typename B, typename T>
typename std::enable_if<std::is_floating_point<B>::value>::type simulateInputAos(T &input, const size_t size) {
  typedef typename T::iterator Iterator;

  //! Creates the random numbers
  std::mt19937 engine(std::random_device{}());
  //! Adjust the random number to a range
  std::uniform_real_distribution<B> random(-1.0f, 1.0f);
  //! For iterating over the input
  Iterator aktElement(input.begin());
  //! The end of the iteration
  Iterator endElement(input.begin() + size);

  while (aktElement != endElement) {
    aktElement->x = random(engine);
    aktElement->y = random(engine);
    aktElement++;
  }
}

//! Creates random numbers for AoS
template <typename B, typename T>
typename std::enable_if<std::is_integral<B>::value>::type simulateInputAos(T &input, const size_t size) {
  typedef typename T::iterator Iterator;

  //! Creates the random numbers
  std::mt19937 engine(std::random_device{}());
  //! Adjust the random number to a range
  std::uniform_int_distribution<B> random(std::numeric_limits<typename T::value_type>::min(), std::numeric_limits<typename T::value_type>::max());
  //! For iterating over the input
  Iterator aktElement(input.begin());
  //! The end of the iteration
  Iterator endElement(input.begin() + size);

  while (aktElement != endElement) {
    aktElement->x = random(engine);
    aktElement->y = random(engine);
    aktElement++;
  }
}

//! AoS with a padding
template<typename T>
void aosWithPadding(benchmark::State &state) {
  //! The size of the values to process
  const size_t inputSize = state.range_x();
  //! The size of the container
  const size_t containerSize =
      (numberOfChunks(inputSize, T::size()) * T::size());

  //! The input and output values for calculation
  VectorCoordinate<typename T::value_type> inputValues(containerSize);
  VectorPolarCoordinate<typename T::value_type> outputValues(containerSize);

  //! Keeps the input values in a vc-vector
  T vCoordinateX;
  T vCoordinateY;
  //! Keeps the output values in a vc-vector
  T vRadius;
  T vPhi;

  //! Creation of input values
  //! The values of the last container are set to 1.0f for the padding
  for (size_t n = 1; n <= T::size(); n++) {
    inputValues[(containerSize - n)].x = 1.0f;
    inputValues[(containerSize - n)].y = 1.0f;
  }
  simulateInputAos<typename T::value_type>(inputValues, inputSize);
  //! Creation of input values completed

  while (state.KeepRunning()) {
    //! Calculation of all and the additional input values
    for (size_t n = 0; n < containerSize; n += T::size()) {
      //! Loads the values to vc-vector
      for (size_t m = 0; m < T::size(); m++) {
        vCoordinateX[m] = inputValues[(n + m)].x;
        vCoordinateY[m] = inputValues[(n + m)].y;
      }

      //! Calculate the polarcoordinates
      std::tie(vRadius, vPhi) = calculatePolarCoordinate(vCoordinateX, vCoordinateY);

      //! Store the values from the vc-vector
      for (size_t m = 0; m < T::size(); m++) {
        outputValues[(n + m)].radius = vRadius[m];
        outputValues[(n + m)].phi = vPhi[m];
      }
    }
  }

  //! Tell the Benchmark how many values are calculated
  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(typename T::value_type));

#ifdef USE_LOG
  std::clog << "Finnished: AoS_Padding\n";
#endif
}

//! AoS unsing interleaved memory with a padding
template <typename T>
void aosWithInterleavedPadding(benchmark::State &state) {
  //! The size of values to process
  const size_t inputSize = state.range_x();
  //! The size of the container
  const size_t containerSize =
      (numberOfChunks(inputSize, T::size()) * T::size());

  //! The input and output values for calculation
  VectorCoordinate<typename T::value_type> inputValues(containerSize);
  VectorPolarCoordinate<typename T::value_type> outputValues(containerSize);

  //! Using interleaved memory for loading
  InterleavedMemoryWrapper<Coordinate<typename T::value_type>, T> wrapperInput(inputValues.data());
  //! Using interleaved memory for writing
  InterleavedMemoryWrapper<PolarCoordinate<typename T::value_type>, T> wrapperOutput(outputValues.data());

  //! Keeps the input values in a vc-vector
  T vCoordinateX;
  T vCoordinateY;
  //! Keeps the output values in a vc-vector
  T vRadius;
  T vPhi;

  //! Creation of input values
  //! The values of the last container are set to 1.0f for the padding
  for (size_t n = 1; n <= T::size(); n++) {
    inputValues[(containerSize - n)].x = 1.0f;
    inputValues[(containerSize - n)].y = 1.0f;
  }

  simulateInputAos<typename T::value_type>(inputValues, inputSize);
  //! Creation of input values completed

  while (state.KeepRunning()) {
    //! Calculation of all and the additional input values
    for (size_t n = 0; n < containerSize; n += (T::size())) {
      //! Loads the values to vc-vector
      Vc::tie(vCoordinateX, vCoordinateY) = wrapperInput[n];

      //! Die Polarkoordinaten berechnen
      std::tie(vRadius, vPhi) = calculatePolarCoordinate(vCoordinateX, vCoordinateY);

      //! Store the values from the vc-vector
      wrapperOutput[n] = Vc::tie(vRadius, vPhi);
    }
  }

  //! Tell the Benchmark how many values are calculated
  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(typename T::value_type));

#ifdef USE_LOG
  std::clog << "Finnished: AoS_Interleaved_Padding\n";
#endif
}

template<typename T>
//! Aos using Gather and Scatter as operator, with a padding
void aosWithGatherScatterPadding(benchmark::State &state) {
typedef typename T::IndexType IT;

  //! The size of the values to process
  const size_t inputSize = state.range_x();
  //! The size of the container
  const size_t containerSize =
      (numberOfChunks(inputSize, T::size()) * T::size());

  //! The input and output values for calculation
  VcVectorCoordinate<typename T::value_type> inputValues(containerSize);
  VcVectorPolarCoordinate<typename T::value_type> outputValues(containerSize);

  //! Keeps the input values in a vc-vector
  T vCoordinateX;
  T vCoordinateY;
  //! Keeps the output values in a vc-vector
  T vRadius;
  T vPhi;

  //! The indexes for gather and scatter
  IT indexes(IT::IndexesFromZero());

  //! Creation of input values
  //! The values of the last container are set to 1.0f for the padding
  for (size_t n = 1; n <= T::size(); n++) {
    inputValues[(containerSize - n)].x = 1.0f;
    inputValues[(containerSize - n)].y = 1.0f;
  }

  simulateInputAos<typename T::value_type>(inputValues, inputSize);
  //! Creation of input values completed

  while (state.KeepRunning()) {
    indexes = IT::IndexesFromZero();

    //! Calculation of all and the additional input values
    for (size_t n = 0; n < containerSize; n += T::size()) {
      //! Loads the values to vc-vector
      vCoordinateX = inputValues[indexes][&Coordinate<typename T::value_type>::x];
      vCoordinateY = inputValues[indexes][&Coordinate<typename T::value_type>::y];

      //! Calculate the polarcoordinates
      std::tie(vRadius, vPhi) = calculatePolarCoordinate(vCoordinateX, vCoordinateY);

      //! Store the values from the vc-vector
      outputValues[indexes][&PolarCoordinate<typename T::value_type>::radius] = vRadius;
      outputValues[indexes][&PolarCoordinate<typename T::value_type>::phi] = vPhi;

      indexes += T::size();
    }
  }

  //! Tell the Benchmark how many values are calculated
  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(typename T::value_type));

#ifdef USE_LOG
  std::clog << "Finnished: AoS_GatherScatter[]_Padding\n";
#endif
}

//! Aos using Gather and Scatter as function, with a padding
template<typename T>
void aosWithGatherScatterUsingFunctionPadding(benchmark::State &state) {
typedef typename T::IndexType IT;

  //! The size of the values to process
  const size_t inputSize = state.range_x();
  //! The size of the container
  const size_t containerSize =
      (numberOfChunks(inputSize, T::size()) * T::size());

  //! The input and output values for calculation
  VectorCoordinate<typename T::value_type> inputValues(containerSize);
  VectorPolarCoordinate<typename T::value_type> outputValues(containerSize);

  //! Keeps the input values in a vc-vector
  T vCoordinateX;
  T vCoordinateY;
  //! Keeps the output values in a vc-vector
  T vRadius;
  T vPhi;

  //! The indexes for gather and scatter
  IT indexes;

  //! Creation of input values
  //! The values of the last container are set to 1.0f for the padding
  for (size_t n = 1; n <= T::size(); n++) {
    inputValues[(containerSize - n)].x = 1.0f;
    inputValues[(containerSize - n)].y = 1.0f;
  }

  simulateInputAos<typename T::value_type>(inputValues, inputSize);
  //! Creation of input values completed

  while (state.KeepRunning()) {
    //! Create the indexes
    indexes[0] = 0;
    for (size_t n = 1; n < T::size(); n++)
      indexes[n] = (n << 1);

    //! Calculation of all and the additional input values
    for (size_t n = 0; n < containerSize; n += T::size()) {
      //! Loads the values to vc-vector
      vCoordinateX.gather(&inputValues[n].x, indexes);
      vCoordinateY.gather(&inputValues[n].y, indexes);

      //! Calculate the polarcoordinates
      std::tie(vRadius, vPhi) = calculatePolarCoordinate(vCoordinateX, vCoordinateY);

      //! Store the values from the vc-vector
      vRadius.scatter(&outputValues[n].radius, indexes);
      vPhi.scatter(&outputValues[n].phi, indexes);
    }
  }

  //! Tell the Benchmark how many values are calculated
  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(typename T::value_type));

#ifdef USE_LOG
  std::clog << "Finnished: AoS_GatherScatter()_Padding\n";
#endif
}

//! AoS with rest scalar
template<typename T>
void aosRestScalar(benchmark::State &state) {
  //! The size of the values to process
  const size_t inputSize = state.range_x();
  //! The size of the values without a full vc-vector
  const size_t missingSize = (inputSize % T::size());

  //! The input and output values for calculation
  VectorCoordinate<typename T::value_type> inputValues(inputSize);
  VectorPolarCoordinate<typename T::value_type> outputValues(inputSize);

  //! Using interleaved memory for loading
  InterleavedMemoryWrapper<Coordinate<typename T::value_type>, T> wrapperInput(inputValues.data());
  //! Using interleaved memory for writing
  InterleavedMemoryWrapper<PolarCoordinate<typename T::value_type>, T> wrapperOutput(outputValues.data());

  //! Keeps the input values in a vc-vector
  T vCoordinateX;
  T vCoordinateY;
  //! Keeps the output values in a vc-vector
  T vRadius;
  T vPhi;

  //! Creation of input values
  simulateInputAos<typename T::value_type>(inputValues, inputSize);
  //! Creation of input values completed

  while (state.KeepRunning()) {
    //! Calculation of the input values without the additional ones
    for (size_t n = 0; n < (inputSize - missingSize); n += T::size()) {
      //! Loads the values to vc-vector
      for (size_t m = 0; m < T::size(); m++) {
        vCoordinateX[m] = inputValues[(n + m)].x;
        vCoordinateY[m] = inputValues[(n + m)].y;
      }

      //! Calculate the polarcoordinates
      std::tie(vRadius, vPhi) = calculatePolarCoordinate(vCoordinateX, vCoordinateY);

      //! Store the values from the vc-vector
      for (size_t m = 0; m < T::size(); m++) {
        outputValues[(n + m)].radius = vRadius[m];
        outputValues[(n + m)].phi = vPhi[m];
      }
    }

    //! Calculate the leftover values scalar
    for (size_t n = (inputSize - missingSize); n < inputSize; n++) {
      //! Scalar calculation
      std::tie(outputValues[n].radius, outputValues[n].phi) =
          calculatePolarCoordinate(inputValues[n].x, inputValues[n].y);
    }
  }

  //! Tell the Benchmark how many values are calculated
  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(typename T::value_type));

#ifdef USE_LOG
  std::clog << "Finnished: AoS_RestScalar\n";
#endif
}

template<typename T>
//! AoS using interleaved memory, with rest scalar
void aosInterleavedRestScalar(benchmark::State &state) {
  //! The size of the values to process
  const size_t inputSize = state.range_x();
  //! The size of the values without a full vc-vector
  const size_t missingSize = (inputSize % T::size());

  //! The input and output values for calculation
  VectorCoordinate<typename T::value_type> inputValues(inputSize);
  VectorPolarCoordinate<typename T::value_type> outputValues(inputSize);

  //! Using interleaved memory for loading
  InterleavedMemoryWrapper<Coordinate<typename T::value_type>, T> wrapperInput(inputValues.data());
  //! Using interleaved memory for writing
  InterleavedMemoryWrapper<PolarCoordinate<typename T::value_type>, T> wrapperOutput(outputValues.data());

  //! Keeps the input values in a vc-vector
  T vCoordinateX;
  T vCoordinateY;
  //! Keeps the output values in a vc-vector
  T vRadius;
  T vPhi;

  //! Creation of input values
  simulateInputAos<typename T::value_type>(inputValues, inputSize);
  //! Creation of input values completed

  while (state.KeepRunning()) {
    //! Calculation of the input values without the additional ones
    for (size_t n = 0; n < (inputSize - missingSize); n += T::size()) {
      //! Loads the values to vc-vector
      Vc::tie(vCoordinateX, vCoordinateY) = wrapperInput[n];

      //! Calculate the polarcoordinates
      std::tie(vRadius, vPhi) = calculatePolarCoordinate(vCoordinateX, vCoordinateY);

      //! Store the values from the vc-vector
      wrapperOutput[n] = Vc::tie(vRadius, vPhi);
    }

    //! Calculate the leftover values scalar
    for (size_t n = (inputSize - missingSize); n < inputSize; n++) {
      //! Scalar calculation
      std::tie(outputValues[n].radius, outputValues[n].phi) =
          calculatePolarCoordinate(inputValues[n].x, inputValues[n].y);
    }
  }

  //! Tell the Benchmark how many values are calculated
  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(typename T::value_type));

#ifdef USE_LOG
  std::clog << "Finnished: AoS_Interleaved_RestScalar\n";
#endif
}

//! AoS using gather and scatter as operator, with rest scalar
template <typename T>
void aosWithGatherScatterRestScalar(benchmark::State &state) {
typedef typename T::IndexType IT;

  //! The size of the values to process
  const size_t inputSize = state.range_x();
  //! The size of the values without a full vc-vector
  const size_t missingSize = (inputSize % T::size());

  //! The input and output values for calculation
  VcVectorCoordinate<typename T::value_type> inputValues(inputSize);
  VcVectorPolarCoordinate<typename T::value_type> outputValues(inputSize);

  //! Keeps the input values in a vc-vector
  T vCoordinateX;
  T vCoordinateY;
  //! Keeps the output in a vc-vector
  T vRadius;
  T vPhi;

  //! The indexes for gather and scatter
  IT indexes(IT::IndexesFromZero());

  //! Creation of input values
  simulateInputAos<typename T::value_type>(inputValues, inputSize);
  //! Creation of input values completed

  while (state.KeepRunning()) {
    indexes = IT::IndexesFromZero();
    //! Calculaton of the input values without the additional ones
    for (size_t n = 0; n < (inputSize - missingSize); n += T::size()) {
      //! Loads the values to vc-vector
      vCoordinateX = inputValues[indexes][&Coordinate<typename T::value_type>::x];
      vCoordinateY = inputValues[indexes][&Coordinate<typename T::value_type>::y];

      //! Calculate the polarcoordinates
      std::tie(vRadius, vPhi) = calculatePolarCoordinate(vCoordinateX, vCoordinateY);

      //! Store the values form the vc-vector
      outputValues[indexes][&PolarCoordinate<typename T::value_type>::radius] = vRadius;
      outputValues[indexes][&PolarCoordinate<typename T::value_type>::phi] = vPhi;

      indexes += T::size();
    }

    //! Calculate the leftover values scaler
    for (size_t n = (inputSize - missingSize); n < inputSize; n++) {
      //! Scalar calculation
      std::tie(outputValues[n].radius, outputValues[n].phi) =
          calculatePolarCoordinate(inputValues[n].x, inputValues[n].y);
    }
  }

  //! Tell the benachmark how many values are calculated
  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(typename T::value_type));

#ifdef USE_LOG
  std::clog << "Finnished: AoS_GatherScatter[]_RestScalar\n";
#endif
}

//! AoS using gather and scatter as function, with rest scalar
template<typename T>
void aosWithGatherScatterAsFunctionRestScalar(benchmark::State &state) {
typedef typename T::IndexType IT;

  //! The size of the values to process
  const size_t inputSize = state.range_x();
  //! The size of the values without a full vc-vector
  const size_t missingSize = (inputSize % T::size());

  //! The input and output values for calaculation
  VcVectorCoordinate<typename T::value_type> inputValues(inputSize);
  VcVectorPolarCoordinate<typename T::value_type> outputValues(inputSize);

  //! Keeps the input values in a vc-vector
  T vCoordinateX;
  T vCoordinateY;
  //! Keeps the output values in a vc-vector
  T vRadius;
  T vPhi;

  //! The indexes for gather and scatter
  IT indexes;

  //! Creation of input values
  simulateInputAos<typename T::value_type>(inputValues, inputSize);
  //! Creation of input values completed

  while (state.KeepRunning()) {
    //! Create the indexes
    indexes[0] = 0;
    for (size_t n = 1; n < T::size(); n++)
      indexes[n] = (n << 1);

    //! Calculation of the input values without the additional ones
    for (size_t n = 0; n < (inputSize - missingSize); n += T::size()) {
      //! Load the values to vc-vector
      vCoordinateX.gather(&inputValues[n].x, indexes);
      vCoordinateY.gather(&inputValues[n].y, indexes);

      //! Calculate the polarcoordinates
      std::tie(vRadius, vPhi) = calculatePolarCoordinate(vCoordinateX, vCoordinateY);

      //! Store the values to vc-vector
      vRadius.scatter(&outputValues[n].radius, indexes);
      vPhi.scatter(&outputValues[n].phi, indexes);
    }

    //! Calculate the leftover values scalar
    for (size_t n = (inputSize - missingSize); n < inputSize; n++) {
      //! Scalar calculation
      std::tie(outputValues[n].radius, outputValues[n].phi) =
          calculatePolarCoordinate(inputValues[n].x, inputValues[n].y);
    }
  }

  //! Tell the benchnmark how many values are calculated
  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(typename T::value_type));

#ifdef USE_LOG
  std::clog << "Finnished: AoS_GatherScatter()_RestScalar\n";
#endif
}
#endif // AOS_H
