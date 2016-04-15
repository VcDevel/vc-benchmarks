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
#ifndef BENCHMARK_H
#define BENCHMARK_H
#include <benchmark/benchmark.h>
#include "benchmarkgenericmemorylayout.h"
#include "registermodification.h"
#include "mathfunctions.h"
#include "vctostring.h"
#include "arithmetics.h"
#include "aos.h"
#include "aovs.h"
#include "soa.h"
#include "additinalcalculations.h"
#include "testvalues.h"

typedef std::unique_ptr<std::vector<::benchmark::internal::Benchmark *>>
    UniqueBenchmarkPointer;
struct TemplateWrapper {

  UniqueBenchmarkPointer benchmarks{
      new std::vector<::benchmark::internal::Benchmark *>()};

  void append(::benchmark::internal::Benchmark *ptr) { benchmarks->push_back(ptr); }

  TemplateWrapper *operator->() { return this; }

  TemplateWrapper &Arg(int x) {
    for (auto &p : *benchmarks) {
      p->Arg(x);
    }

    return *this;
  }

  TemplateWrapper &Range(int start, int limit) {
    for (auto &p : *benchmarks) {
      p->Range(start, limit);
    }

    return *this;
  }

  TemplateWrapper &DenseRange(int start, int limit) {
    for (auto &p : *benchmarks) {
      p->DenseRange(start, limit);
    }

    return *this;
  }

  TemplateWrapper &ArgPair(int x, int y) {
    for (auto &p : *benchmarks) {
      p->ArgPair(x, y);
    }

    return *this;
  }

  TemplateWrapper &RangePair(int lo1, int hi1, int lo2, int hi2) {
    for (auto &p : *benchmarks) {
      p->RangePair(lo1, hi1, lo2, hi2);
    }

    return *this;
  }

  template <typename... Ts> TemplateWrapper &Apply(Ts &&... args) {
    for (auto &p : *benchmarks) {
      p->Apply(args...);
    }

    return *this;
  }

  TemplateWrapper &MinTime(double t) {
    for (auto &p : *benchmarks) {
      p->MinTime(t);
    }

    return *this;
  }

  TemplateWrapper &UseRealTime() {
    for (auto &p : *benchmarks) {
      p->UseRealTime();
    }

    return *this;
  }

  TemplateWrapper &Threads(int t) {
    for (auto &p : *benchmarks) {
      p->Threads(t);
    }

    return *this;
  }

  TemplateWrapper &ThreadRange(int min_threads, int max_threads) {
    for (auto &p : *benchmarks) {
      p->ThreadRange(min_threads, max_threads);
    }

    return *this;
  }

  TemplateWrapper &ThreadPerCpu() {
    for (auto &p : *benchmarks) {
      p->ThreadPerCpu();
    }

    return *this;
  }

  operator int() { return 0; }
};

#define Vc_BENCHMARK_TEMPLATE(n_, ...)                                                   \
  template <unsigned int N>                                                              \
  TemplateWrapper BENCHMARK_PRIVATE_CONCAT(typeListFunc, n_, __LINE__)() {               \
    std::string name(#n_);                                                               \
    name.append("<");                                                                    \
    name.append(typeToString<__VA_ARGS__::at<N>>());                                     \
    name.append(">");                                                                    \
    TemplateWrapper wrapper =                                                            \
        std::move(BENCHMARK_PRIVATE_CONCAT(typeListFunc, n_, __LINE__) < N - 1 > ());    \
    wrapper->append(::benchmark::internal::RegisterBenchmarkInternal(                    \
        new ::benchmark::internal::FunctionBenchmark(name.c_str(),                       \
                                                     n_<__VA_ARGS__::at<N>>)));          \
    return wrapper;                                                                      \
  }                                                                                      \
                                                                                         \
  template <>                                                                            \
  TemplateWrapper BENCHMARK_PRIVATE_CONCAT(typeListFunc, n_, __LINE__)<0u>() {           \
    std::string name(#n_);                                                               \
    name.append("<");                                                                    \
    name.append(typeToString<__VA_ARGS__::at<0>>());                                     \
    name.append(">");                                                                    \
    TemplateWrapper wrapper;                                                             \
                                                                                         \
    wrapper.append(::benchmark::internal::RegisterBenchmarkInternal(                     \
        new ::benchmark::internal::FunctionBenchmark(name.c_str(),                       \
                                                     n_<__VA_ARGS__::at<0>>)));          \
    return wrapper;                                                                      \
  }                                                                                      \
  int BENCHMARK_PRIVATE_CONCAT(variable, n_, __LINE__) =                                 \
      BENCHMARK_PRIVATE_CONCAT(typeListFunc, n_, __LINE__)<__VA_ARGS__::size() - 1>()

#define Vc_SCALAR_INT_VECTORS                                                            \
  Typelist<Vc::Scalar::int_v, Vc::Scalar::uint_v, Vc::Scalar::short_v,                   \
           Vc::Scalar::ushort_v>
#define Vc_SCALAR_FLOAT_VECTORS Typelist<Vc::Scalar::double_v, Vc::Scalar::float_v>
#define Vc_SCALAR_VECTORS concat<Vc_SCALAR_FLOAT_VECTORS, Vc_SCALAR_INT_VECTORS>

#ifdef Vc_IMPL_SSE
#define Vc_SSE_INT_VECTORS                                                               \
  Typelist<Vc::SSE::int_v, Vc::SSE::uint_v, Vc::SSE::short_v, Vc::SSE::ushort_v>
#define Vc_SSE_FLOAT_VECTORS Typelist<Vc::SSE::double_v, Vc::SSE::float_v>
#define Vc_SSE_VECTORS concat<Vc_SSE_INT_VECTORS, Vc_SSE_FLOAT_VECTORS>
#else
#define Vc_SSE_INT_VECTORS Typelist<>
#define Vc_SSE_FLOAT_VECTORS Typelist<>
#define Vc_SSE_VECTORS Typelist<>
#endif // Vc_IMPL_SSE

#ifdef Vc_IMPL_AVX
#define Vc_AVX_INT_VECTORS                                                               \
  Typelist<Vc::AVX::int_v, Vc::AVX::uint_v, Vc::AVX::short_v, Vc::AVX::ushort_v>
#define Vc_AVX_FLOAT_VECTORS Typelist<Vc::AVX::double_v, Vc::AVX::float_v>
#define Vc_AVX_VECTORS Vc_AVX_FLOAT_VECTORS
#else
#define Vc_AVX_INT_VECTORS Typelist<>
#define Vc_AVX_FLOAT_VECTORS Typelist<>
#define Vc_AVX_VECTORS Typelist<>
#endif // Vc_IMPL_AVX

#ifdef Vc_IMPL_MIC
#define Vc_MIC_INT_VECTORS                                                               \
  Typelist<Vc::MIC::int_v, Vc::MIC::uint_v, Vc::MIC::short_v, Vc::MIC::ushort_v>
#define Vc_MIC_FLOAT Typelist<Vc::MIC::double_v, Vc::MIC::float_v>
#define Vc_MIC_VECTORS concat<Vc_MIC_INT_VECTORS, Vc_MIC_FLOAT_VECTORS>
#else
#define Vc_MIC_INT_VECTORS Typelist<>
#define Vc_MIC_FLOAT_VECTORS Typelist<>
#define Vc_MIC_VECTORS Typelist<>
#endif // Vc_IMPL_MIC

#define Vc_ALL_VECTORS                                                                   \
  concat<Vc_SCALAR_VECTORS, Vc_SSE_VECTORS, Vc_AVX_VECTORS, Vc_MIC_VECTORS>

#define Vc_ALL_INT_VECTORS                                                               \
  concat<Vc_SCALAR_INT_VECTORS, Vc_SSE_INT_VECTORS, Vc_AVX_INT_VECTORS,                  \
         Vc_MIC_INT_VECTORS>
#define Vc_ALL_FLOAT_VECTORS                                                             \
  concat<Vc_SCALAR_FLOAT_VECTORS, Vc_SSE_FLOAT_VECTORS, Vc_AVX_FLOAT_VECTORS,            \
         Vc_MIC_FLOAT_VECTORS>

#define Vc_ALL_MEMORY_LAYOUT_TESTS                                                       \
  concat<outer_product<Typelist<AovsAccess, Baseline>, Typelist<Padding>>,               \
         outer_product<                                                                  \
             Typelist<AosSubscriptAccess, InterleavedAccess, AosGatherScatterAccess,     \
                      SoaSubscriptAccess, LoadStoreAccess, SoaGatherScatterAccess>,      \
             Typelist<Padding, RestScalar>>>

#define Vc_ALL_BASIC_ARITHMETICS                                                         \
  Typelist<Addition, Substraction, Multiplication, Division>

#endif // BENCHMARK_H
