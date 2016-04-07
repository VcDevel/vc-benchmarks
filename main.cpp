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
#define USE_GOOGLE
//#define USE_LOG

//#define BENCHMARKING_DATA_LAYOUT
#define BENCHMARKING_ADDITION

#include "BenchmarkHelper.h"
#include "Mathfunctions.h"

#include "Addition.h"

#include "AoS.h"
#include "AovS.h"
#include "SoA.h"
#include "AdditinalCalculations.h"
#include <Vc/cpuid.h>
#include <typelist.h>

#ifndef USE_GOOGLE
//! Is only used for testing
int main() {
    std::cout <<

     << "\n";
    return 0;
}
#else
using Vc::CpuId;

//! Tests the L1-Cache sizes dynamicly
void dynamicL1CacheSize(benchmark::internal::Benchmark *function) {
    size_t n;

    CpuId::init();
    for(n = 1; n < (float_v::size() + 1); n += 2) {
        function->Arg(n);
    }

    function->Range((float_v::size()*2), CpuId::L1Data());
}

//! Tests the L2-Cache sizes dynamicly
void dynamicL2CacheSize(benchmark::internal::Benchmark *function) {
    CpuId::init();

    function->Arg(CpuId::L1Data());
    function->Arg(CpuId::L2Data() >> 2);

    function->Arg(CpuId::L2Data());
}

//! Tests the L3-Cache sizes dynamicly
void dynamicL3CacheSize(benchmark::internal::Benchmark *function) {
    CpuId::init();

    function->Range(CpuId::L2Data(), CpuId::L3Data());
}

//! Tests the L1-Cache sizes from my PC
void sizesToL1CacheSize(benchmark::internal::Benchmark *function) {
  int sizes[] = {1,    2,    3,    4,    6,    8,    12,   16,   26,   32,
                 41,   53,   64,   70,   88,   109,  128,  177,  201,  256,
                 356,  566,  789,  1024, 1234, 1323, 1600, 1899, 2048, 2934,
                 3433, 3677, 4096, 4304, 4666, 4900, 5555, 7809, 8000, 8192};
  size_t n;

  for (n = 0; n < (sizeof(sizes) / sizeof(int)); n++) {
    function->Arg(sizes[n]);
  }
}

//! Tests the L2-Cache sizes from my PC
void sizesToL2CacheSize(benchmark::internal::Benchmark *function) {
  int sizes[] = {8192,  9999,  12333, 14444, 16384, 18770, 22456, 26666, 30233,
                 32768, 38622, 42333, 48665, 53535, 60066, 62000, 65536};
  size_t n;

  for (n = 0; n < (sizeof(sizes) / sizeof(int)); n++) {
    function->Arg(sizes[n]);
  }
}

//! Tests the L3-Cache sizes from my PC
void sizesToL3CacheSize(benchmark::internal::Benchmark *function) {
  int sizes[] = {65536,  85066,  102023, 131072, 155295, 262144,
                 434595, 524288, 600345, 768000, 834402, 1048576};
  size_t n;

  for (n = 0; n < (sizeof(sizes) / sizeof(int)); n++) {
    function->Arg(sizes[n]);
  }
}

//! Test series for the baseline, for more datas
void baselineSeries(benchmark::internal::Benchmark *function) {
  for (int n = 1; n <= 256000; n *= 2) {
    function->Arg(n);
  }
}

void (*applyFunction)(benchmark::internal::Benchmark *) = dynamicL2CacheSize;
static std::vector<void (*)()> helper;

#ifdef BENCHMARKING_DATA_LAYOUT
//! Scalar
BENCHMARK(Scalar)->Apply(applyFunction)->UseRealTime();

//! Start of AoS
BENCHMARK(AoS_Padding)->Apply(applyFunction)->UseRealTime();
BENCHMARK(AoS_RestScalar)->Apply(applyFunction)->UseRealTime();

BENCHMARK(AoS_Interleaved_Padding)->Apply(applyFunction)->UseRealTime();
BENCHMARK(AoS_Interleaved_RestScalar)->Apply(applyFunction)->UseRealTime();

BENCHMARK(AoS_GatherScatter_Padding)->UseRealTime()->Apply(applyFunction)->UseRealTime();
BENCHMARK(AoS_GatherScatter_RestScalar)->Apply(applyFunction)->UseRealTime();

BENCHMARK(AoS_GatherScatterFunc_Padding)->Apply(applyFunction)->UseRealTime();
BENCHMARK(AoS_GatherScatterFunc_RestScalar)->Apply(applyFunction)->UseRealTime();

//! AovS
BENCHMARK(AovS)->Apply(applyFunction)->UseRealTime();

//! Start of SoA
BENCHMARK(SoA_Padding)->Apply(applyFunction)->UseRealTime();
BENCHMARK(SoA_RestScalar)->Apply(applyFunction)->UseRealTime();

BENCHMARK(SoA_LoadStore_Padding)->Apply(applyFunction)->UseRealTime();
BENCHMARK(SoA_LoadStore_RestScalar)->Apply(applyFunction)->UseRealTime();

BENCHMARK(SoA_GatherScatter_Padding)->Apply(applyFunction)->UseRealTime();
BENCHMARK(SoA_GatherScatter_RestScalar)->Apply(applyFunction)->UseRealTime();

BENCHMARK(SoA_GatherScatterFunc_Padding)->Apply(applyFunction)->UseRealTime();
BENCHMARK(SoA_GatherScatterFunc_RestScalar)->Apply(applyFunction)->UseRealTime();

//! Baseline
BENCHMARK(baselineCalculation)->Apply(applyFunction)->UseRealTime();

#endif // BENCHMARKING_DATA_LAYOUT

#ifdef BENCHMARKING_ADDITION

#define Vc_BENCHMARK_TEMPLATE(n, ...)           \
  BENCHMARK_PRIVATE_DECLARE(n) =             \
      (::benchmark::internal::RegisterBenchmarkInternal( \
        new ::benchmark::internal::FunctionBenchmark( \
        #n "<" "Wuff D:" ">", n<__VA_ARGS__::at<0>::at<0>>)))

#define Vc_BENCHMARK_TEMPLATE_IMPROVED(n, x, ...)           \
  BENCHMARK_PRIVATE_DECLARE(n) =             \
      (::benchmark::internal::RegisterBenchmarkInternal( \
        new ::benchmark::internal::FunctionBenchmark( \
        #n "<" #__VA_ARGS__::at<0> ">", n<__VA_ARGS__::at<x>>)))

#define Vc_BENCHMARK_TEMPLATE_PLANSCHI(n, ...)       \
template<unsigned int N>                              \
int BENCHMARK_PRIVATE_CONCAT(typeListFunc, n, __LINE__)() {                                 \
BENCHMARK_PRIVATE_DECLARE(n) =                          \
      (::benchmark::internal::RegisterBenchmarkInternal( \
        new ::benchmark::internal::FunctionBenchmark(     \
        #n "<" #__VA_ARGS__ ">", n<__VA_ARGS__::at<N>>))); \
                                                            \
        BENCHMARK_PRIVATE_CONCAT(typeListFunc, n, __LINE__)<N - 1>();                             \
        return 0;                                             \
}                                                              \
                                                                \
template<>                                                       \
int BENCHMARK_PRIVATE_CONCAT(typeListFunc, n, __LINE__)<0u>()                                          \
{                                                                  \
BENCHMARK_PRIVATE_DECLARE(n) =                                      \
      (::benchmark::internal::RegisterBenchmarkInternal(             \
        new ::benchmark::internal::FunctionBenchmark(                 \
        #n "<" #__VA_ARGS__ ">", n<__VA_ARGS__::at<0>>)));             \
        return 0;                                                       \
}                                                                        \
int BENCHMARK_PRIVATE_CONCAT(variable, n, __LINE__) = BENCHMARK_PRIVATE_CONCAT(typeListFunc, n, __LINE__)<__VA_ARGS__::size() - 1>();

/*(void (*)(benchmark::State&)) (*functionPtr)() = []() {
                                return &additionVectorVector<int_v>;
                             };*/

/*template<typename T>
::benchmark::internal::Benchmark* zws(const char *name) {
    std::cout << typeid(T).name() << " :3 " << name << "\n";

    return nullptr;//::benchmark::internal::RegisterBenchmarkInternal(new ::benchmark::internal::FunctionBenchmark(additionVectorVector<T::at<0>>));
}*/


//Was Matthias will:
//Vc_BENCHMARK(additionVectorVector, ALL_VECTOR_TYPES)->UseRealTime();
//typelist.h in Vc soll helfen

#ifdef Vc_IMPL_SSE
Vc_BENCHMARK_TEMPLATE_PLANSCHI(additionVectorVector,
    Typelist<int_v, float_v>)//->UseRealTime());

Vc_BENCHMARK_TEMPLATE_PLANSCHI(additionVectorVector,
    Typelist<int_v, float_v>)//->UseRealTime());
//Vc_BENCHMARK_TEMPLATE(additionVectorVector, Vc::SSE::int_v)->UseRealTime();
#endif

#ifdef Vc_IMPL_AVX
//Vc_BENCHMARK_TEMPLATE(additionVectorVector, Vc::AVX::double_v)->UseRealTime();
//Vc_BENCHMARK_TEMPLATE(additionVectorVector, Vc::AVX::int_v)->UseRealTime();
#endif

/*#ifdef Vc_IMPL_MIC
BENCHMARK_TEMPLATE(additionVectorVector, Vc::MIC::double_v)->UseRealTime();
#endif // Vc_IMPL_MIC
#ifdef Vc_IMPL_AVX
BENCHMARK_TEMPLATE(additionVectorVector, Vc::AVX::double_v)->UseRealTime();
BENCHMARK_TEMPLATE(additionVectorVector, Vc::SSE::double_v)->UseRealTime();

BENCHMARK_TEMPLATE(additionVectorVector, Vc::Scalar::double_v)->UseRealTime();*/
#endif

//BENCHMARK_MAIN();
int main(int argc, char **argv) {
    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
}
#endif // USE_GOOGLE

