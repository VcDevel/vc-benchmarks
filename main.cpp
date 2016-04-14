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
#include "benchmark.h"




#ifdef BENCHMARKING_DATA_LAYOUT
//! Scalar
BENCHMARK(Scalar)->Apply(applyFunction)->UseRealTime();

//! Start of AoS
BENCHMARK(aosWithPadding)->Apply(applyFunction)->UseRealTime();
BENCHMARK(aosWithRestScalar)->Apply(applyFunction)->UseRealTime();

BENCHMARK(aosWithInterleavedPadding)->Apply(applyFunction)->UseRealTime();
BENCHMARK(aosWithInterleavedRestScalar)->Apply(applyFunction)->UseRealTime();

BENCHMARK(aosWithGatherScatterPadding)
    ->UseRealTime()
    ->Apply(applyFunction)
    ->UseRealTime();
BENCHMARK(aosWithGatherScatterRestScalar)->Apply(applyFunction)->UseRealTime();

BENCHMARK(aosWithGatherScatterAsFunctionPadding)->Apply(applyFunction)->UseRealTime();
BENCHMARK(aosWithGatherScatterAsFunctionRestScalar)->Apply(applyFunction)->UseRealTime();

//! AovS
BENCHMARK(aovs)->Apply(applyFunction)->UseRealTime();

//! Start of SoA
BENCHMARK(soaWithPadding)->Apply(applyFunction)->UseRealTime();
BENCHMARK(soaWithRestScalar)->Apply(applyFunction)->UseRealTime();

BENCHMARK(soaWithLoadStorePadding)->Apply(applyFunction)->UseRealTime();
BENCHMARK(soaWithLoadStoreRestScalar)->Apply(applyFunction)->UseRealTime();

BENCHMARK(soaWithGatherScatterPadding)->Apply(applyFunction)->UseRealTime();
BENCHMARK(soaWithGatherScatterRestScalar)->Apply(applyFunction)->UseRealTime();

BENCHMARK(soaWithGatherScatterAsFunctionPadding)->Apply(applyFunction)->UseRealTime();
BENCHMARK(soaWithGatherScatterAsFunctionRestScalar)->Apply(applyFunction)->UseRealTime();

//! Baseline
BENCHMARK(baseline)->Apply(applyFunction)->UseRealTime();

#endif // BENCHMARKING_DATA_LAYOUT

#ifdef BENCHMARKING_ADDITION
Vc_BENCHMARK_TEMPLATE_PLANSCHI(additionVectorVector, (Vc_ALL_VECTORS));
#endif // BENCHMARKING_ADDITION

//Vc_BENCHMARK_TEMPLATE_PLANSCHI(additionVectorVector, Vc_AVX_VECTORS)->Range(1, 12345)->DenseRange(1, 5)->ArgPair(0, 23)->RangePair(0, 2, 0, 4)->MinTime(2.0)->UseRealTime()->Threads(2);

//OUTER_PRODUCT WIRD HELFEN :3
//Vc_BENCHMARK_TEMPLATE_PLANSCHI(aosNormalWith, Vc_AVX_VECTORS, PaddingPolicyPadding);

/*BENCHMARK_TEMPLATE(aosWithPadding, float_v)->Arg(1234);
BENCHMARK_TEMPLATE(aosWithInterleavedPadding, float_v)->Arg(1234);
BENCHMARK_TEMPLATE(aosWithGatherScatterPadding, float_v)->Arg(1234);*/

/*BENCHMARK_TEMPLATE(aosWithRestScalar, float_v)->Arg(1234);
BENCHMARK_TEMPLATE(aosWithInterleavedRestScalar, float_v)->Arg(1234);
BENCHMARK_TEMPLATE(aosWithGatherScatterRestScalar, float_v)->Arg(1234);*/
/*
BENCHMARK_TEMPLATE(veryMelone, float_v, AosSubscriptAccess, RestScalar)->Arg(1234);
BENCHMARK_TEMPLATE(veryMelone, float_v, InterleavedAccess, RestScalar)->Arg(1234);
BENCHMARK_TEMPLATE(veryMelone, float_v, AosGatherScatterAccess, RestScalar)->Arg(1234);
BENCHMARK_TEMPLATE(veryMelone, float_v, AosSubscriptAccess, Padding)->Arg(1234);
BENCHMARK_TEMPLATE(veryMelone, float_v, InterleavedAccess, Padding)->Arg(1234);
BENCHMARK_TEMPLATE(veryMelone, float_v, AosGatherScatterAccess, Padding)->Arg(1234);

BENCHMARK_TEMPLATE(veryMelone, float_v, AovsAccess, Padding)->Arg(1234);

BENCHMARK_TEMPLATE(veryMelone, float_v, SoaSubscriptAccess, RestScalar)->Arg(1234);
BENCHMARK_TEMPLATE(veryMelone, float_v, LoadStoreAccess, RestScalar)->Arg(1234);
BENCHMARK_TEMPLATE(veryMelone, float_v, SoaGatherScatterAccess, RestScalar)->Arg(1234);
BENCHMARK_TEMPLATE(veryMelone, float_v, SoaSubscriptAccess, Padding)->Arg(1234);
BENCHMARK_TEMPLATE(veryMelone, float_v, LoadStoreAccess, Padding)->Arg(1234);
BENCHMARK_TEMPLATE(veryMelone, float_v, SoaGatherScatterAccess, Padding)->Arg(1234);*/

Vc_BENCHMARK_TEMPLATE_PLANSCHI(veryMelone,
    outer_product<Vc_AVX_VECTORS, Vc_ALL_MEMORY_LAYOUT_TESTS>
)->Arg(1234);

/*BENCHMARK_TEMPLATE(aosWithPadding, Vc::double_v, ->Apply(applyFunction));
BENCHMARK_TEMPLATE(aosWithInterleavedPadding, Vc::double_v)->Apply(applyFunction);
BENCHMARK_TEMPLATE(aosWithGatherScatterPadding, Vc::double_v)->Apply(applyFunction);
BENCHMARK_TEMPLATE(aosWithGatherScatterUsingFunctionPadding, Vc::double_v)->Apply(applyFunction);
BENCHMARK_TEMPLATE(aosRestScalar, Vc::double_v)->Apply(applyFunction);
BENCHMARK_TEMPLATE(aosInterleavedRestScalar, Vc::double_v)->Apply(applyFunction);
BENCHMARK_TEMPLATE(aosWithGatherScatterRestScalar, Vc::double_v)->Apply(applyFunction);
BENCHMARK_TEMPLATE(aosWithGatherScatterAsFunctionRestScalar, Vc::double_v)->Apply(applyFunction);

BENCHMARK_TEMPLATE(aovs, Vc::double_v)->Apply(applyFunction);

BENCHMARK_TEMPLATE(soaWithPadding, Vc::double_v)->Apply(applyFunction);
BENCHMARK_TEMPLATE(soaWithLoadStorePadding, Vc::double_v)->Apply(applyFunction);
BENCHMARK_TEMPLATE(soaWithGatherScatterPadding, Vc::double_v)->Apply(applyFunction);
BENCHMARK_TEMPLATE(soaWithGatherScatterAsFunctionPadding, Vc::double_v)->Apply(applyFunction);
BENCHMARK_TEMPLATE(soaWithRestScalar, Vc::double_v)->Apply(applyFunction);
BENCHMARK_TEMPLATE(soaWithLoadStoreRestScalar, Vc::double_v)->Apply(applyFunction);
BENCHMARK_TEMPLATE(soaWithGatherScatterRestScalar, Vc::double_v)->Apply(applyFunction);
BENCHMARK_TEMPLATE(soaWithGatherScatterFunctionRestScalar, Vc::double_v)->Apply(applyFunction);*/
