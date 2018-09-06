/*{{{
Copyright © 2018 GSI Helmholtzzentrum für Schwerionenforschung GmbH
                 Matthias Kretz <kretz@kde.org>

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
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

}}}*/

#include "benchmark.h"
#include <random>

std::default_random_engine urbg{std::random_device()()};

struct SincosInput {
  template <class T>
  typename std::enable_if<std::is_floating_point<T>::value, T>::type random_input()
      const {
    std::uniform_real_distribution<T> dist(0, 2000 * M_PI);
    return dist(urbg);
  }

  template <class T>
  typename std::enable_if<Vc::is_simd_vector<T>::value, T>::type random_input()
      const {
    std::uniform_real_distribution<typename T::value_type> dist(0, 2 * M_PI);
    T v(dist(urbg));
    for (size_t i = 1; i < T::size(); ++i) {
      v[i] += 0.01f;
    }
    return v;
  }
};

struct Sin : SincosInput {
  static constexpr std::size_t count = 1;

  template <class T> T operator()(const T &x) const {
    using std::sin;
    return sin(x);
  }
};

struct Cos : SincosInput {
  static constexpr std::size_t count = 1;

  template <class T> T operator()(const T &x) const {
    using std::cos;
    return cos(x);
  }
};

struct Sincos : SincosInput {
  static constexpr std::size_t count = 2;

  std::pair<float, float> operator()(float x) const {
    std::pair<float, float> r;
    ::sincosf(x, &r.first, &r.second);
    return r;
  }
  std::pair<double, double> operator()(double x) const {
    std::pair<double, double> r;
    ::sincos(x, &r.first, &r.second);
    return r;
  }
  template <class T> std::pair<T, T> operator()(const T &x) const {
    std::pair<T, T> r;
    Vc::sincos(x, &r.first, &r.second);
    return r;
  }
};

template <class Setup> void _(benchmark::State &state) {
  using Operation = typename Setup::template at<0>;
  using ArgType = typename Setup::template at<1>;

  const Operation op;
  ArgType x = op.template random_input<ArgType>();

  for (auto _ : state) {
    x += 0.0001f;
    do_not_optimize(op(x));
  }
  state.counters["Rate"] =
      state.iterations() * element_count<ArgType>::value * op.count;
}

Vc_BENCHMARK_TEMPLATE(
    _,
    outer_product<Typelist<Sin, Cos, Sincos>,
                  concat<float, all_vectors_of<float>, double, all_vectors_of<double>>>);
