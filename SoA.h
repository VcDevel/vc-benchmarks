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
#ifndef SOA_H
#define SOA_H
#include <vector>
#include <Vc/vector>

using Vc::int_v;

typedef std::vector<float, Vc::Allocator<float>> vectorFloat;
typedef Vc::vector<float, Vc::Allocator<float>> VCvectorFloat;

//! The type of the indices for gather and scatter
typedef float_v::IndexType IT;

//! Stores all coordinates with an array
template <typename T> struct ArrayOfCoordinates {
  T x;
  T y;
};

//! Stores all polarcoordinates with an array
template <typename T> struct ArrayOfPolarCoordinates {
  T radius;
  T phi;
};

typedef ArrayOfCoordinates<vectorFloat> StdArrayOfCoordinates;
typedef ArrayOfCoordinates<VCvectorFloat> VcArrayOfCoordinates;

typedef ArrayOfPolarCoordinates<vectorFloat> StdArrayOfPolarCoordinates;
typedef ArrayOfPolarCoordinates<VCvectorFloat> VcArrayOfPolarCoordinates;

//! Creates random numbers for SoA
template <typename T>
void simulateInput_SoA(ArrayOfCoordinates<T> &input, const size_t size) {
  //! Creates the random numbers
  std::default_random_engine engine(time(nullptr));
  //! Adjust the random number to a range
  std::uniform_real_distribution<float> random(-1.0f, 1.0f);
  size_t n;

  for (n = 0; n < size; n++) {
    input.x[n] = random(engine);
    input.y[n] = random(engine);
  }
}

//! SoA with a padding
void SoA_Padding(benchmark::State &state) {
  //! The label for the plotter
  const std::string label(getLabelString("SoA_Padding/", state.range_x(), 1));
  //! The size of the values to process
  const size_t inputSize = state.range_x();
  //! The size of the container
  const size_t containerSize =
      (numberOfChunks(inputSize, float_v::size()) * float_v::size());

  //! The input and output values for calculation
  StdArrayOfCoordinates inputValues;
  StdArrayOfPolarCoordinates outputValues;
  size_t n, m;

  //! Keeps the input values in a vc-vector
  float_v coordinateX_v;
  float_v coordinateY_v;
  //! Keeps the output values in a vc-vector
  float_v radius_v;
  float_v phi_v;

  //! Change container size
  inputValues.x.reserve(containerSize);
  inputValues.y.reserve(containerSize);

  outputValues.radius.reserve(containerSize);
  outputValues.phi.reserve(containerSize);

  //! Creation of input values
  //! The values of the last container are set to 1.0f for the padding
  for (n = 1; n <= float_v::size(); n++) {
    inputValues.x[(containerSize - n)] = 1.0f;
    inputValues.y[(containerSize - n)] = 1.0f;
  }

  simulateInput_SoA(inputValues, inputSize);
  //! Creation of input values completed

  while (state.KeepRunning()) {
    //! Calculation of all and the additional input values
    for (n = 0; n < containerSize; n += float_v::size()) {
      //! Loads the values to vc-vector
      for (m = 0; m < float_v::size(); m++) {
        coordinateX_v[m] = inputValues.x[(n + m)];
        coordinateY_v[m] = inputValues.y[(n + m)];
      }

      //! Calculate the polarcoordinates
      std::tie(radius_v, phi_v) = calcularePolarCoordinate(coordinateX_v, coordinateY_v);

      //! Store the values from the vc-vector
      for (m = 0; m < float_v::size(); m++) {
        outputValues.radius[(n + m)] = radius_v[m];
        outputValues.phi[(n + m)] = phi_v[m];
      }
    }
  }

  //! Tell the Benchmark how many values are calculated
  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(float));

  //! Set the label
  state.SetLabel(label);

#ifdef USE_LOG
  std::clog << "Finnished: " << label << "\n";
#endif
}

//! SoA using load and store, with a padding
void SoA_LoadStore_Padding(benchmark::State &state) {
  //! The label for the plotter
  const std::string label(getLabelString("SoA_LoadStore_Padding/", state.range_x(), 1));
  //! The size of the values to process
  const size_t inputSize = state.range_x();
  //! The size of the container
  const size_t containerSize =
      (numberOfChunks(inputSize, float_v::size()) * float_v::size());

  //! The input and output values for calculation
  StdArrayOfCoordinates inputValues;
  StdArrayOfPolarCoordinates outputValues;
  size_t n;

  //! Keeps the input values in a vc-vector
  float_v coordinateX_v;
  float_v coordinateY_v;
  //! Keeps the output values in a vc-vector
  float_v radius_v;
  float_v phi_v;

  //! Change container size
  inputValues.x.reserve(containerSize);
  inputValues.y.reserve(containerSize);

  outputValues.radius.reserve(containerSize);
  outputValues.phi.reserve(containerSize);

  //! Creation of input values
  //! The values of the last container are set to 1.0f for the padding
  for (n = 1; n <= float_v::size(); n++) {
    inputValues.x[(containerSize - n)] = 1.0f;
    inputValues.y[(containerSize - n)] = 1.0f;
  }

  simulateInput_SoA(inputValues, inputSize);
  //! Creation of input values completed

  while (state.KeepRunning()) {
    //! Calculation of all and the additional input values
    for (n = 0; n < containerSize; n += float_v::size()) {
      //! Loads the values to vc-vector
      coordinateX_v.load((inputValues.x.data() + n));
      coordinateY_v.load((inputValues.y.data() + n));

      //! Calculate the polarcoordinates
      std::tie(radius_v, phi_v) = calcularePolarCoordinate(coordinateX_v, coordinateY_v);

      //! Store the values from the vc-vector
      radius_v.store((outputValues.radius.data() + n));
      phi_v.store((outputValues.phi.data() + n));
    }
  }

  //! Tell the Benchmark how many values are calculated
  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(float));

  //! Set the label
  state.SetLabel(label);

#ifdef USE_LOG
  std::clog << "Finnished: " << label << "\n";
#endif
}

//! SoA using gather and scatter as operator, with a padding
void SoA_GatherScatter_Padding(benchmark::State &state) {
  //! The label for the plotter
  const std::string label(
      getLabelString("SoA_GatherScatter[]_Padding/", state.range_x(), 1));
  //! The size of the values to process
  const size_t inputSize = state.range_x();
  //! The size of the container
  const size_t containerSize =
      (numberOfChunks(inputSize, float_v::size()) * float_v::size());

  //! The input and output values for calculation
  VcArrayOfCoordinates inputValues;
  VcArrayOfPolarCoordinates outputValues;
  size_t n;

  //! Keeps the input values in a vc-vector
  float_v coordinateX_v;
  float_v coordinateY_v;
  //! Keeps the output values in a vc-vector
  float_v radius_v;
  float_v phi_v;
  //! The indexes for gather and scatter
  IT indexes(IT::IndexesFromZero());

  //! Change container size
  inputValues.x.reserve(containerSize);
  inputValues.y.reserve(containerSize);

  outputValues.radius.reserve(containerSize);
  outputValues.phi.reserve(containerSize);

  //! Creation of input values
  //! The values of the last container are set to 1.0f for the padding
  for (n = 1; n <= float_v::size(); n++) {
    inputValues.x[(containerSize - n)] = 1.0f;
    inputValues.y[(containerSize - n)] = 1.0f;
  }

  simulateInput_SoA(inputValues, inputSize);
  //! Creation of input values completed

  while (state.KeepRunning()) {
    //! Calculation of all and the additional input values
    indexes = IT::IndexesFromZero();
    for (n = 0; n < containerSize; n += float_v::size()) {
      //! Loads the values to vc-vector
      coordinateX_v = inputValues.x[indexes];
      coordinateY_v = inputValues.y[indexes];

      //! Calculate the polarcoordinates
      std::tie(radius_v, phi_v) = calcularePolarCoordinate(coordinateX_v, coordinateY_v);

      //! Store the values from the vc-vector
      outputValues.radius[indexes] = radius_v;
      outputValues.phi[indexes] = phi_v;
      indexes += float_v::size();
    }
  }

  //! Tell the Benchmark how many values are calculated
  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(float));

  //! Set the label
  state.SetLabel(label);

#ifdef USE_LOG
  std::clog << "Finnished: " << label << "\n";
#endif
}

//! SoA using gather and scatter as function, with a padding
void SoA_GatherScatterFunc_Padding(benchmark::State &state) {
  //! The label for the plotter
  const std::string label(
      getLabelString("SoA_GatherScatter()_Padding/", state.range_x(), 1));
  //! The size of the values to process
  const size_t inputSize = state.range_x();
  //! The size of the container
  const size_t containerSize =
      (numberOfChunks(inputSize, float_v::size()) * float_v::size());

  //! The input and output values for calculation
  StdArrayOfCoordinates inputValues;
  StdArrayOfPolarCoordinates outputValues;
  size_t n;

  //! Keeps the input values in a vc-vector
  float_v coordinateX_v;
  float_v coordinateY_v;
  //! Keeps the output values in a vc-vector
  float_v radius_v;
  float_v phi_v;
  //! The indexes for gather and scatter
  IT indexes(IT::IndexesFromZero());

  //! Change container size
  inputValues.x.reserve(containerSize);
  inputValues.y.reserve(containerSize);

  outputValues.radius.reserve(containerSize);
  outputValues.phi.reserve(containerSize);

  //! Creation of input values
  //! The values of the last container are set to 1.0f for the padding
  for (n = 1; n <= float_v::size(); n++) {
    inputValues.x[(containerSize - n)] = 1.0f;
    inputValues.y[(containerSize - n)] = 1.0f;
  }

  simulateInput_SoA(inputValues, inputSize);
  //! Creation of input values completed

  while (state.KeepRunning()) {
    indexes = IT::IndexesFromZero();
    //! Calculation of all input values including the additional input values
    for (n = 0; n < containerSize; n += float_v::size()) {
      //! Loads the values to vc-vector
      coordinateX_v.gather(inputValues.x.data(), indexes);
      coordinateY_v.gather(inputValues.y.data(), indexes);

      //! Calculate the polarcoordinates
      std::tie(radius_v, phi_v) = calcularePolarCoordinate(coordinateX_v, coordinateY_v);

      //! Store the values from the vc-vector
      radius_v.scatter(outputValues.radius.data(), indexes);
      phi_v.scatter(outputValues.phi.data(), indexes);
      indexes += float_v::size();
    }
  }

  //! Tell the Benchmark how many values are calculated
  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(float));

  //! Set the label
  state.SetLabel(label);

#ifdef USE_LOG
  std::clog << "Finnished: " << label << "\n";
#endif
}

//! SoA with rest scalar
void SoA_RestScalar(benchmark::State &state) {
  //! The label for the plotter
  const std::string label(getLabelString("SoA_RestScalar/", state.range_x(), 1));
  //! The size of the values to process
  const size_t inputSize = state.range_x();
  //! The size of the values without a full vc-vector
  const size_t missingSize = (inputSize % float_v::size());

  //! The input and output values for calculation
  StdArrayOfCoordinates inputValues;
  StdArrayOfPolarCoordinates outputValues;
  size_t n, m;

  //! Keeps the input values in a vc-vector
  float_v coordinateX_v;
  float_v coordinateY_v;
  //! Keeps the output values in a vc-vector
  float_v radius_v;
  float_v phi_v;

  //! Change container size
  inputValues.x.reserve(inputSize);
  inputValues.y.reserve(inputSize);

  outputValues.radius.reserve(inputSize);
  outputValues.phi.reserve(inputSize);

  //! Creation of input values
  simulateInput_SoA(inputValues, inputSize);
  //! Creation of input values completed

  while (state.KeepRunning()) {
    //! Calculation of the input values without the additional ones
    for (n = 0; n < (inputSize - missingSize); n += float_v::size()) {
      //! Loads the values to vc-vector
      for (m = 0; m < float_v::size(); m++) {
        coordinateX_v[m] = inputValues.x[(n + m)];
        coordinateY_v[m] = inputValues.y[(n + m)];
      }

      //! Calculate the polarcoordinates
      std::tie(radius_v, phi_v) = calcularePolarCoordinate(coordinateX_v, coordinateY_v);

      //! Store the values from the vc-vector
      for (m = 0; m < float_v::size(); m++) {
        outputValues.radius[(n + m)] = radius_v[m];
        outputValues.phi[(n + m)] = phi_v[m];
      }
    }

    //! Calculate the leftover values scalar
    for (n = (inputSize - missingSize); n < inputSize; n++) {
      //! Scalar Calculation
      std::tie(outputValues.radius[n], outputValues.phi[n]) =
          calcularePolarCoordinate(inputValues.x[n], inputValues.y[n]);
    }
  }

  //! Tell the Benchmark how many values are calculated
  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(float));

  //! Set the label
  state.SetLabel(label);

#ifdef USE_LOG
  std::clog << "Finnished: " << label << "\n";
#endif
}

//! SoA using load and Store, with rest scalar
void SoA_LoadStore_RestScalar(benchmark::State &state) {
  //! The label for the plotter
  const std::string label(
      getLabelString("SoA_LoadStore_RestScalar/", state.range_x(), 1));
  //! The size of the values to process
  const size_t inputSize = state.range_x();
  //! The size of the values without a full vc-vector
  const size_t missingSize = (inputSize % float_v::size());

  //! The input and output values for calculation
  StdArrayOfCoordinates inputValues;
  StdArrayOfPolarCoordinates outputValues;
  size_t n;

  //! Keeps the input values in a vc-vector
  float_v coordinateX_v;
  float_v coordinateY_v;
  //! Keeps the output values in a vc-vector
  float_v radius_v;
  float_v phi_v;

  //! Change container size
  inputValues.x.reserve(inputSize);
  inputValues.y.reserve(inputSize);

  outputValues.radius.reserve(inputSize);
  outputValues.phi.reserve(inputSize);

  //! Creation of input values
  simulateInput_SoA(inputValues, inputSize);
  //! Creation of input values completed

  while (state.KeepRunning()) {
    //! Calculation of the input values without the additional ones
    for (n = 0; n < (inputSize - missingSize); n += float_v::size()) {
      //! Loads the values to vc-vector
      coordinateX_v.load((inputValues.x.data() + n));
      coordinateY_v.load((inputValues.y.data() + n));

      //! Calculate the polarcoordinates
      std::tie(radius_v, phi_v) = calcularePolarCoordinate(coordinateX_v, coordinateY_v);

      //! Store the values from the vc-vector
      radius_v.store((outputValues.radius.data() + n));
      phi_v.store((outputValues.phi.data() + n));
    }

    //! Calculate the leftover values scalar
    for (n = (inputSize - missingSize); n < inputSize; n++) {
      //! Scalar calculation
      std::tie(outputValues.radius[n], outputValues.phi[n]) =
          calcularePolarCoordinate(inputValues.x[n], inputValues.y[n]);
    }
  }

  //! Tell the Benchmark how many values are calculated
  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(float));

  //! Set the label
  state.SetLabel(label);

#ifdef USE_LOG
  std::clog << "Finnished: " << label << "\n";
#endif
}

//! SoA using gather and scatter as operator, with rest scalar
void SoA_GatherScatter_RestScalar(benchmark::State &state) {
  //! The label for the plotter
  const std::string label(
      getLabelString("SoA_GatherScatter[]_RestScalar/", state.range_x(), 1));
  //! The size of the values to process
  const size_t inputSize = state.range_x();
  //! The size of the values without a full vc-vector
  const size_t missingSize = (inputSize % float_v::size());

  //! The input and output values for calculation
  StdArrayOfCoordinates inputValues;
  StdArrayOfPolarCoordinates outputValues;
  size_t n;

  //! Keeps the input values in a vc-vector
  float_v coordinateX_v;
  float_v coordinateY_v;
  //! Keeps the output values in a vc-vector
  float_v radius_v;
  float_v phi_v;
  //! The indexes for gather and scatter
  IT indexes(IT::IndexesFromZero());

  //! Change container size
  inputValues.x.reserve(inputSize);
  inputValues.y.reserve(inputSize);

  outputValues.radius.reserve(inputSize);
  outputValues.phi.reserve(inputSize);

  //! Creation of input values
  simulateInput_SoA(inputValues, inputSize);
  //! Creation of input values completed

  while (state.KeepRunning()) {
    indexes = IT::IndexesFromZero();
    //! Calculation of the input values without the additional ones
    for (n = 0; n < (inputSize - missingSize); n += float_v::size()) {
      //! Loads the values to vc-vector
      coordinateX_v.gather(inputValues.x.data(), indexes);
      coordinateY_v.gather(inputValues.y.data(), indexes);

      //! Calculate the polarcoordinates
      std::tie(radius_v, phi_v) = calcularePolarCoordinate(coordinateX_v, coordinateY_v);

      //! Store the values from the vc-vector
      radius_v.scatter(outputValues.radius.data(), indexes);
      phi_v.scatter(outputValues.phi.data(), indexes);
      indexes += float_v::size();
    }

    //! Calculation of the input values without the additional ones
    for (n = (inputSize - missingSize); n < inputSize; n++) {
      //! Scalar calculation
      std::tie(outputValues.radius[n], outputValues.phi[n]) =
          calcularePolarCoordinate(inputValues.x[n], inputValues.y[n]);
    }
  }

  //! Tell the Benchmark how many values are calculated
  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(float));

  //! Set the label
  state.SetLabel(label);

#ifdef USE_LOG
  std::clog << "Finnished: " << label << "\n";
#endif
}

//! SoA using gather and scatter as function, with rest scalar
void SoA_GatherScatterFunc_RestScalar(benchmark::State &state) {
  //! The label for the plotter
  const std::string label(
      getLabelString("SoA_GatherScatter()_RestScalar/", state.range_x(), 1));
  //! The size of the values to process
  const size_t inputSize = state.range_x();
  //! The size of the values without a full vc-vector
  const size_t missingSize = (inputSize % float_v::size());

  //! The input and output values for calculation
  VcArrayOfCoordinates inputValues;
  VcArrayOfPolarCoordinates outputValues;
  size_t n;

  //! Keeps the input values in a vc-vector
  float_v coordinateX_v;
  float_v coordinateY_v;
  //! Keeps the output values in a vc-vector
  float_v radius_v;
  float_v phi_v;
  //! The indexes for gather and scatter
  IT indexes(IT::IndexesFromZero());

  //! Change container size
  inputValues.x.reserve(inputSize);
  inputValues.y.reserve(inputSize);

  outputValues.radius.reserve(inputSize);
  outputValues.phi.reserve(inputSize);

  //! Creation of input values
  simulateInput_SoA(inputValues, inputSize);
  //! Creation of input values completed

  while (state.KeepRunning()) {
    indexes = IT::IndexesFromZero();
    //! Calculation of the input values without the additional ones
    for (n = 0; n < (inputSize - missingSize); n += float_v::size()) {
      //! Loads the values to vc-vector
      coordinateX_v.gather(inputValues.x.data(), indexes);
      coordinateY_v.gather(inputValues.y.data(), indexes);

      //! Calculate the polarcoordinates
      std::tie(radius_v, phi_v) = calcularePolarCoordinate(coordinateX_v, coordinateY_v);

      //! Store the values from the vc-vector
      radius_v.scatter(outputValues.radius.data(), indexes);
      phi_v.scatter(outputValues.phi.data(), indexes);
      indexes += float_v::size();
    }

    //! Calculate the leftover values scalar
    for (n = (inputSize - missingSize); n < inputSize; n++) {
      //! Scalar calculation
      std::tie(outputValues.radius[n], outputValues.phi[n]) =
          calcularePolarCoordinate(inputValues.x[n], inputValues.y[n]);
    }
  }

  //! Tell the Benchmark how many values are calculated
  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(float));

  //! Set the label
  state.SetLabel(label);

#ifdef USE_LOG
  std::clog << "Finnished: " << label << "\n";
#endif
}
#endif // SOA_H
