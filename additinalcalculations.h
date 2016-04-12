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
#ifndef ADDITINAL_CALCULATIONS_H
#define ADDITINAL_CALCULATIONS_H

//! For measuring the time of the polarcoordinate calculation
template<typename T>
void baseline(benchmark::State &state) {
  size_t containerSize = numberOfChunks(state.range_x(), T::size());

  T vCoordinateX(T::Random());
  T vCoordinateY(T::Random());

  T vRadius;
  T vPhi;

  while (state.KeepRunning()) {
    for (size_t n = 0; n < containerSize; n++) {

      fakeMemoryModification(vCoordinateX);
      fakeMemoryModification(vCoordinateY);

      std::tie(vRadius, vPhi) = calculatePolarCoordinate(vCoordinateX, vCoordinateY);

      fakeRegisterRead(vRadius);
      fakeRegisterRead(vPhi);
    }
  }

  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(float));

#ifdef USE_LOG
  std::clog << "Finnished: Baseline\n";
#endif
}

//! Scalar
template<typename T>
typename std::enable_if<std::is_fundamental<T>::value>::type scalar(benchmark::State &state) {
  const size_t inputSize = state.range_x();

  VectorCoordinate<T> inputValues(inputSize);
  VectorPolarCoordinate<T> outputValues(inputSize);

  simulateInputAos(inputValues, inputSize);

  while (state.KeepRunning()) {
    for (size_t n = 0; n < inputSize; n++) {

      std::tie(outputValues[n].radius, outputValues[n].phi) =
          calculatePolarCoordinate(inputValues[n].x, inputValues[n].y);
    }
  }

  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(T));

#ifdef USE_LOG
  std::clog << "Finnished: Scaler\n";
#endif
}
#endif // ADDITINAL_CALCULATIONS_H
