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
#ifndef AOVS_H
#define AOVS_H
#include <vector>

template<typename T>
using VectorizedCoordinateContainer = std::vector<Coordinate<T>, Vc::Allocator<Coordinate<T>>>;
template<typename T>
using PolarCoordinateContainer = std::vector<PolarCoordinate<T>, Vc::Allocator<PolarCoordinate<T>>>;

template<typename T>
void simulateInputAovs(VectorizedCoordinateContainer<T> &input, const size_t size) {
  typename VectorizedCoordinateContainer<T>::iterator aktElement = input.begin();
  typename VectorizedCoordinateContainer<T>::iterator endElement = (input.begin() + size);

  while (aktElement != endElement) {
      aktElement->x = T::Random();
      aktElement->y = T::Random();

      aktElement++;
  }
}

template<typename T>
struct AovsLayout {
    using IC = VectorizedCoordinateContainer<T>;
    using OC = PolarCoordinateContainer<T>;

    IC inputValues;
    OC outputValues;

    AovsLayout(size_t containerSize)
        : inputValues(containerSize/T::size()), outputValues(containerSize/T::size())
    {
        simulateInputAovs<T>(inputValues, inputValues.size());
    }

    Coordinate<typename T::value_type> coordinate(size_t index) {
        Coordinate<typename T::value_type> r;
        return r;
    }

    void setPolarCoordinate(size_t index, const PolarCoordinate<typename T::value_type> &coord) {
    }
};

template<typename T>
struct AovsAccessImpl : public AovsLayout<T> {

    AovsAccessImpl(size_t containerSize) : AovsLayout<T>(containerSize) {
    }

    void setupLoop() {
    }

    Coordinate<T> load(size_t index) {
        return AovsLayout<T>::inputValues[index/T::size()];
    }

    void store(size_t index, const PolarCoordinate<T> &coord) {
        AovsLayout<T>::outputValues[index/T::size()] = coord;
    }
};

struct AovsAccess {
    template<typename T> using type = AovsAccessImpl<T>;
};
#endif // AOVS_H
