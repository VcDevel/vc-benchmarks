/*
Copyright © 2016-2018 Matthias Kretz <kretz@kde.org>
Copyright © 2016 Björn Gaier

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

template <typename T> struct BaselineLayout {
  using IC = Coordinate<T>;
  using OC = PolarCoordinate<T>;

  IC inputValue;
  OC outputValue;

  BaselineLayout(size_t containerSize) {
    inputValue.x = T::Random();
    inputValue.y = T::Random();
  }

  Coordinate<typename T::value_type> coordinate(size_t index) {
    Coordinate<typename T::value_type> r;
    return r;
  }

  void setPolarCoordinate(size_t index,
                          const PolarCoordinate<typename T::value_type> &coord) {}
};

template <typename T> struct BaselineImpl : public BaselineLayout<T> {
  BaselineImpl(size_t containerSize) : BaselineLayout<T>(containerSize) {}

  void setupLoop() {}

  Coordinate<T> load(size_t index) {
    fake_modification(BaselineLayout<T>::inputValue.x);
    fake_modification(BaselineLayout<T>::inputValue.y);

    return BaselineLayout<T>::inputValue;
  }

  void store(size_t index, const PolarCoordinate<T> &coord) {
    BaselineLayout<T>::outputValue = coord;

    do_not_optimize(BaselineLayout<T>::outputValue.radius);
    do_not_optimize(BaselineLayout<T>::outputValue.phi);
  }
};

struct Baseline {
  template <typename T> using type = BaselineImpl<T>;
};

#endif // ADDITINAL_CALCULATIONS_H
