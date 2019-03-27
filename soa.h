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

template <typename T>
using ArrayOfCoordinates = Coordinate<Vc::vector<T, Vc::Allocator<T>>>;
template <typename T>
using ArrayOfPolarCoordinates = PolarCoordinate<Vc::vector<T, Vc::Allocator<T>>>;

//! Creates random numbers for SoA
template <typename B, typename T>
void simulateInputSoa(ArrayOfCoordinates<T> &input, const size_t size) {
  using Dist = typename std::conditional<std::is_integral<B>::value,
                                         std::uniform_int_distribution<B>,
                                         std::uniform_real_distribution<B>>::type;
  std::mt19937 engine(std::random_device{}());
  Dist random(std::numeric_limits<B>::min(), std::numeric_limits<B>::max());

  for (size_t n = 0; n < size; n++) {
    input.x[n] = random(engine);
    input.y[n] = random(engine);
  }
}

template <typename T> struct SoaLayout {
  using TY = typename T::value_type;
  using IC = ArrayOfCoordinates<typename T::value_type>;
  using OC = ArrayOfPolarCoordinates<typename T::value_type>;

  IC inputValues;
  OC outputValues;

  SoaLayout(size_t containerSize) {
    inputValues.x.reserve(containerSize);
    inputValues.y.reserve(containerSize);

    outputValues.radius.reserve(containerSize);
    outputValues.phi.reserve(containerSize);
  }

  Coordinate<TY> coordinate(size_t index) {
    Coordinate<TY> r;

    r.x = inputValues.x[index];
    r.y = inputValues.y[index];

    return r;
  }

  void setPolarCoordinate(size_t index, const PolarCoordinate<TY> &coord) {
    outputValues.radius[index] = coord.radius;
    outputValues.phi[index] = coord.phi;
  }
};

template <typename T> struct SoaSubscriptAccessImpl : public SoaLayout<T> {
  SoaSubscriptAccessImpl(size_t containerSize) : SoaLayout<T>(containerSize) {}

  void setupLoop() {}

  Coordinate<T> load(size_t index) {
    Coordinate<T> r;

    for (size_t m = 0; m < T::size(); m++) {
      r.x[m] = SoaLayout<T>::inputValues.x[(index + m)];
      r.y[m] = SoaLayout<T>::inputValues.y[(index + m)];
    }

    return r;
  }

  void store(size_t index, const PolarCoordinate<T> &coord) {
    for (size_t m = 0; m < T::size(); m++) {
      SoaLayout<T>::outputValues.radius[(index + m)] = coord.radius[m];
      SoaLayout<T>::outputValues.phi[(index + m)] = coord.phi[m];
    }
  }
};

template <typename T> struct LoadStoreAccessImpl : public SoaLayout<T> {
  LoadStoreAccessImpl(size_t containerSize) : SoaLayout<T>(containerSize) {}

  void setupLoop() {}

  Coordinate<T> load(size_t index) {
    Coordinate<T> r;

    r.x.load(SoaLayout<T>::inputValues.x.data() + index, Vc::Aligned);
    r.y.load(SoaLayout<T>::inputValues.y.data() + index, Vc::Aligned);

    return r;
  }

  void store(size_t index, const PolarCoordinate<T> &coord) {

    coord.radius.store((SoaLayout<T>::outputValues.radius.data() + index), Vc::Aligned);
    coord.phi.store((SoaLayout<T>::outputValues.phi.data() + index), Vc::Aligned);
  }
};

template <typename T> struct SoaGatherScatterAccessImpl : public SoaLayout<T> {
  typedef typename T::IndexType IT;

  IT indexes;

  SoaGatherScatterAccessImpl(size_t containerSize)
      : SoaLayout<T>(containerSize), indexes(IT([](int n) { return n; })) {}

  void setupLoop() {
    indexes = IT([](int n) { return n; });
  }

  Coordinate<T> load(size_t index) {
    Coordinate<T> r;

    r.x = SoaLayout<T>::inputValues.x[indexes];
    r.y = SoaLayout<T>::inputValues.y[indexes];

    return r;
  }

  void store(size_t index, const PolarCoordinate<T> &coord) {
    SoaLayout<T>::outputValues.radius[indexes] = coord.radius;
    SoaLayout<T>::outputValues.phi[indexes] = coord.phi;
    indexes += T::size();
  }
};

struct SoaSubscriptAccess {
  template <typename T> using type = SoaSubscriptAccessImpl<T>;
};

struct LoadStoreAccess {
  template <typename T> using type = LoadStoreAccessImpl<T>;
};

struct SoaGatherScatterAccess {
  template <typename T> using type = SoaGatherScatterAccessImpl<T>;
};
#endif // SOA_H
