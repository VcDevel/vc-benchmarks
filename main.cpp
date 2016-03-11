#define USE_GOOGLE
//#undef USE_GOOGLE
#define USE_LOG
//#undef USE_LOG

#include "BenchmarkHelper.h"
#include "VectorizationHelper.h"

#include "AoS.h"
#include "AovS.h"
#include "SoA.h"
#include "Sonst.h"

#ifndef USE_GOOGLE
//!Is only used for testing
int main()
{
    SoA_GatherScatter_Padding();
}
#else

void sizesToL1CacheSize(benchmark::internal::Benchmark *function)
{
 int sizes[] = {1, 2, 3, 4, 6, 8, 12, 16, 26, 32, 41, 53, 64, 70, 88, 109, 128, 177, 201, 256, 356, 566, 789, 1024, 1234, 1323, 1600, 1899, 2048, 2934, 3433, 3677, 4096, 4304, 4666, 4900, 5555, 7809, 8000, 8192};
 size_t n;

        for(n = 0; n < (sizeof(sizes)/sizeof(int)); n++)
        {
            function->Arg(sizes[n]);
        }
}

void (*applyFunction)(benchmark::internal::Benchmark*) = sizesToL1CacheSize;

//!Scalar
//BENCHMARK(Scalar)                           ->Apply(applyFunction);

//!Start of AoS
BENCHMARK(AoS_Padding)                      ->Apply(applyFunction);
BENCHMARK(AoS_RestScalar)                   ->Apply(applyFunction);

BENCHMARK(AoS_Interleaved_Padding)          ->Apply(applyFunction);
BENCHMARK(AoS_Interleaved_RestScalar)       ->Apply(applyFunction);

BENCHMARK(AoS_GatherScatter_Padding)        ->Apply(applyFunction);
BENCHMARK(AoS_GatherScatter_RestScalar)     ->Apply(applyFunction);

BENCHMARK(AoS_GatherScatterFunc_Padding)    ->Apply(applyFunction);
BENCHMARK(AoS_GatherScatterFunc_RestScalar) ->Apply(applyFunction);

//!AovS
BENCHMARK(AovS)                             ->Apply(applyFunction);

//!Start of SoA
BENCHMARK(SoA_Padding)                      ->Apply(applyFunction);
BENCHMARK(SoA_RestScalar)                   ->Apply(applyFunction);

BENCHMARK(SoA_LoadStore_Padding)            ->Apply(applyFunction);
BENCHMARK(SoA_LoadStore_RestScalar)         ->Apply(applyFunction);

BENCHMARK(SoA_GatherScatter_Padding)        ->Apply(applyFunction);
BENCHMARK(SoA_GatherScatter_RestScalar)     ->Apply(applyFunction);

BENCHMARK(SoA_GatherScatterFunc_Padding)    ->Apply(applyFunction);
BENCHMARK(SoA_GatherScatterFunc_RestScalar) ->Apply(applyFunction);

//!Baseline
BENCHMARK(baselineCalculation)              ->Apply(applyFunction);

BENCHMARK_MAIN();
#endif
