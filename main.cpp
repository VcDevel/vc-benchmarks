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

//!Tests the L1-Cache sizes from my PC
void sizesToL1CacheSize(benchmark::internal::Benchmark *function)
{
 int sizes[] = {1, 2, 3, 4, 6, 8, 12, 16, 26, 32, 41, 53, 64, 70, 88, 109, 128, 177, 201, 256, 356, 566, 789, 1024, 1234, 1323, 1600, 1899, 2048, 2934, 3433, 3677, 4096, 4304, 4666, 4900, 5555, 7809, 8000, 8192};
 size_t n;

        for(n = 0; n < (sizeof(sizes)/sizeof(int)); n++)
        {
            function->Arg(sizes[n]);
        }
}

//!Tests the L2-Cache sizes from my PC
void sizesToL2CacheSize(benchmark::internal::Benchmark *function)
{
 int sizes[] = {8192, 9999, 12333, 14444, 16384, 18770, 22456, 26666, 30233, 32768, 38622, 42333, 48665, 53535, 60066, 62000, 65536};
 size_t n;

    for(n = 0; n < (sizeof(sizes)/sizeof(int)); n++)
    {
        function->Arg(sizes[n]);
    }
}

//!Tests the L3-Cache sizes from my PC
void sizesToL3CacheSize(benchmark::internal::Benchmark *function)
{
 int sizes[] = {65536, 85066, 102023,  131072, 155295, 262144, 434595, 524288, 600345, 768000, 834402, 1048576};
 size_t n;

    for(n = 0; n < (sizeof(sizes)/sizeof(int)); n++)
    {
        function->Arg(sizes[n]);
    }
}

void (*applyFunction)(benchmark::internal::Benchmark*) = sizesToL3CacheSize;

//!Scalar
BENCHMARK(Scalar)                           ->Apply(applyFunction);

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
