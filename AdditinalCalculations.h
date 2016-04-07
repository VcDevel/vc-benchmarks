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
#ifndef SONST_H
#define SONST_H

using Vc::float_v;

//! Creates random numbers
void simulateInput_v(float_v &inputX, float_v &inputY) {
  //! Creates the random numbers
  std::mt19937 engine(std::random_device{}());
  //! Adjust the random number to a range
  std::uniform_real_distribution<float> random(-1.0f, 1.0f);
  size_t n;

  for (n = 0; n < float_v::size(); n++) {
    inputX[n] = random(engine);
    inputY[n] = random(engine);
  }
}

//! For measuring the time of the polarcoordinate calculation
void baselineCalculation(benchmark::State &state) {
  //! The size of the container
  size_t containerSize = numberOfChunks(state.range_x(), float_v::size());

  //! Keeps the input values in a vc-vector
  float_v coordinateX;
  float_v coordinateY;

  //! Keeps the output values in a vc-vector
  float_v radius;
  float_v phi;

  size_t n;

  //! Creation of input values
  simulateInput_v(coordinateX, coordinateY);
  //! Creation of input values completed

  while (state.KeepRunning()) {
    for (n = 0; n < containerSize; n++) {
      //! Prevent the optimizer from optimizing
      //asm volatile("" : "+m"(coordinateX), "+m"(coordinateY));
      fakeMemoryModification(coordinateX); fakeMemoryModification(coordinateY);
      //! Calculates only one value
      std::tie(radius, phi) = calculatePolarCoordinate(coordinateX, coordinateY);
      //asm volatile("" ::"x"(radius), "x"(phi));
      fakeRegisterRead(radius); fakeRegisterRead(phi);
    }
  }

  //! Tell the benchnmark how many values are calculated
  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(float));

#ifdef USE_LOG
  std::clog << "Finnished: Baseline\n";
#endif
}

#include <Vc/cpuid.h>

//! Scalar
void Scalar(benchmark::State &state) {
  //! The size of the input values
  const size_t inputSize = state.range_x();

  //! The input and output values for calculation
  vectorCoordinate inputValues(inputSize);
  vectorPolarCoordinate outputValues(inputSize);
  size_t n;

  //! Creation of input values
  simulateInput_AoS(inputValues, inputSize);
  //! Creation of input values completed

  while (state.KeepRunning()) {
    //! Calculate alle values
    for (n = 0; n < inputSize; n++) {
      //! Scalar Calculation
      std::tie(outputValues[n].radius, outputValues[n].phi) =
          calculatePolarCoordinate(inputValues[n].x, inputValues[n].y);
    }
  }

  //! Tell the benchmark how many values are calculates
  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(float));

#ifdef USE_LOG
  std::clog << "Finnished: Scaler\n";
#endif
}
#endif // SONST_H
