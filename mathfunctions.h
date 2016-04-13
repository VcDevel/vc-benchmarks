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
#ifndef MATH_FUNCTIONS_H
#define MATH_FUNCTIONS_H
#include <math.h>
#include <tuple>
#include <Vc/Vc>

using Vc::float_v;
using Vc::sqrt;
using Vc::atan2;

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

template <typename T>
inline std::tuple<T, T> calculatePolarCoordinate(const T &x, const T &y) {
  return std::make_tuple(sqrt(((x * x) + (y * y))), atan2(y, x) * 57.295780181884765625f);
}

template<typename T>
inline PolarCoordinate<T> calculatePolarCoordinate(const Coordinate<T> &coord) {
    PolarCoordinate<T> r;

    r.radius = sqrt((coord.x * coord.x) + (coord.y * coord.y));
    r.phi    = atan2(coord.y, coord.x) * 57.295780181884765625f;

    return r;
}

constexpr size_t numberOfChunks(size_t inputSize, size_t chunkSize) {
  return (inputSize + chunkSize - 1) / chunkSize;
}
#endif // MATH_FUNCTIONS_H
