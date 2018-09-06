/*{{{
Copyright © 2016-2018 Matthias Kretz <kretz@kde.org>

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

#include <numeric>
#include "benchmark.h"

struct Unrolled {};
struct Simple {};

template <typename T>
inline void workLoop(const T *input, T *output, const int N, Simple)
{
  using V = Vc::Vector<T>;
  for (int i = 0; i < N; i += V::size()) {
    (V(&input[i], Vc::Aligned) + 1).store(&output[i], Vc::Aligned);
  }
}

template <typename T>
inline void workLoop(const T *input, T *output, const int N, Unrolled)
{
  using V = Vc::Vector<T>;
  V x[4];
  x[0] = (V(&input[0], Vc::Aligned) + 1);
  x[1] = (V(&input[    V::size()], Vc::Aligned) + 1);
  x[2] = (V(&input[2 * V::size()], Vc::Aligned) + 1);
  x[3] = (V(&input[3 * V::size()], Vc::Aligned) + 1);
  for (int i = 4 * V::size(); i < N; i += 4 * V::size()) {
    x[0].store(&output[i - 4 * V::size()], Vc::Aligned);
    x[0] = (V(&input[i], Vc::Aligned) + 1);
    x[1].store(&output[i - 3 * V::size()], Vc::Aligned);
    x[1] = (V(&input[i +     V::size()], Vc::Aligned) + 1);
    x[2].store(&output[i - 2 * V::size()], Vc::Aligned);
    x[2] = (V(&input[i + 2 * V::size()], Vc::Aligned) + 1);
    x[3].store(&output[i - 1 * V::size()], Vc::Aligned);
    x[3] = (V(&input[i + 3 * V::size()], Vc::Aligned) + 1);
  }
  x[0].store(&output[N - 4 * V::size()], Vc::Aligned);
  x[1].store(&output[N - 3 * V::size()], Vc::Aligned);
  x[2].store(&output[N - 2 * V::size()], Vc::Aligned);
  x[3].store(&output[N - 1 * V::size()], Vc::Aligned);
}

template <typename Config> void loop(benchmark::State &state)
{
  using V = typename Config::template at<0>;
  using Unroll = typename Config::template at<1>;
  using T = typename V::EntryType;
  using Container = std::vector<T, Vc::Allocator<T>>;
  constexpr int N = 16 * 1024;
  Container  input(N);
  Container output(N);
  std::iota(std::begin(input), std::end(input), 0);
  for (auto _ : state) {
    workLoop(&input[0], &output[0], N, Unroll());
  }
  const double items = state.iterations() * N;
  state.counters["Items"] = items;
  state.counters["Bytes"] = items * sizeof(T);
}

Vc_BENCHMARK_TEMPLATE(loop, outer_product<all_real_vectors, Typelist<Simple, Unrolled>>);
