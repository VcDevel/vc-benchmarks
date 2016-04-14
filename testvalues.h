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
#ifndef TEST_VALUES_H
#define TEST_VALUES_H

#include <Vc/cpuid.h>

using Vc::CpuId;

//! Tests the L1-Cache sizes dynamicly
void dynamicL1CacheSize(benchmark::internal::Benchmark *function) {
  CpuId::init();
  for (size_t n = 1; n < (float_v::size() + 1); n += 2) {
    function->Arg(n);
  }

  function->Range((float_v::size() * 2), CpuId::L1Data());
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

//! Tests all cache sizes
void dynamicAllCacheSize(benchmark::internal::Benchmark *function) {
  CpuId::init();

  function->Range(1, CpuId::L3Data());
}

#endif // TEST_VALUES_H
