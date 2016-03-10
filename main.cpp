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

//!Configuration will be added soon

BENCHMARK_MAIN();
#endif
