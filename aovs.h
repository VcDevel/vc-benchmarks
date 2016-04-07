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
#ifndef AOVS_H
#define AOVS_H
#include <vector>

//! Stores coordinates with vectorized variables
struct VectorizedCoordinate {
  float_v vX;
  float_v vY;
};

//! Stores polarcoordinate with vectorized variables
struct VectorizedPolarCoordinate {
  float_v vRadius;
  float_v vPhi;
};

typedef std::vector<VectorizedCoordinate, Vc::Allocator<VectorizedCoordinate>>
    VectorVectorizedCoordinate;
typedef std::vector<VectorizedPolarCoordinate, Vc::Allocator<VectorizedPolarCoordinate>>
    VectorVectorizedPolarCoordinate;

//! Creates random numbers for AovS
void simulateInputAovs(VectorVectorizedCoordinate &input, const size_t size) {
  //! For iterating over the input
  VectorVectorizedCoordinate::iterator aktElement = input.begin();
  //! The end of the iteration
  VectorVectorizedCoordinate::iterator endElement = (input.begin() + size);

  while (aktElement != endElement) {
    for (size_t m = 0; m < float_v::size(); m++) {
      aktElement->vX = float_v::Random();
      aktElement->vY = float_v::Random();
    }

    aktElement++;
  }
}

//! AovS
void aovs(benchmark::State &state) {
  //! The size of the values to process
  const size_t inputSize = state.range_x();
  //! The size of the container
  size_t containerSize = numberOfChunks(inputSize, float_v::size());

  //! The input and output values for calculation
  VectorVectorizedCoordinate inputValues(containerSize);
  VectorVectorizedPolarCoordinate outputValues(containerSize);

  //! Creation of input values
  simulateInputAovs(inputValues, containerSize);
  for (size_t n = 0; n < float_v::size(); n++) {
    inputValues[(containerSize - 1)].vX[n] = 1.0f;
    inputValues[(containerSize - 1)].vY[n] = 1.0f;
  }
  //! Creation of input values completed

  while (state.KeepRunning()) {
    //! Calculation of all the input values
    for (size_t n = 0; n < containerSize; n++) {
      std::tie(outputValues[n].vRadius, outputValues[n].vPhi) =
          calculatePolarCoordinate(inputValues[n].vX, inputValues[n].vY);
    }
  }

  //! Tell the Benchmark how many values are calculated
  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(float));

#ifdef USE_LOG
  std::clog << "Finnished: AovS\n";
#endif
}
#endif // AOVS_H
