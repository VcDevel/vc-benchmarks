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
#include "mathfunctions.h"
#include "aos.h"
#include "aovs.h"
#include "soa.h"
#include "additinolcalculations.h"
#include <Vc/cpuid.h>

//! Tests all cache sizes
void dynamicAllCacheSize(benchmark::internal::Benchmark *function) {
  Vc::CpuId::init();

  function->Range(1, Vc::CpuId::L3Data());
}

struct RestScalar {};
struct Padding {};

#define Vc_ALL_MEMORY_LAYOUT_TESTS                                                       \
  concat<outer_product<Typelist<AovsAccess, Baseline>, Typelist<Padding>>,               \
         outer_product<                                                                  \
             Typelist<AosSubscriptAccess, InterleavedAccess, AosGatherScatterAccess,     \
                      SoaSubscriptAccess, LoadStoreAccess, SoaGatherScatterAccess>,      \
             Typelist<Padding, RestScalar>>>

template <typename TT> inline void benchmarkGenericMemoryLayout(benchmark::State &state) {
  using T = typename TT::template at<0>;
  using A = typename TT::template at<1>;
  using B = typename TT::template at<2>;

  typedef typename A::template type<T> P;

  const size_t inputSize = state.range_x();
  const size_t missingSize =
      std::is_same<B, RestScalar>::value ? inputSize % T::size() : 0;
  const size_t containerSize = std::is_same<B, RestScalar>::value
                                   ? inputSize - missingSize
                                   : numberOfChunks(inputSize, T::size()) * T::size();

  P magic(containerSize + missingSize);

  while (state.KeepRunning()) {
    magic.setupLoop();

    for (size_t n = 0; n < containerSize; n += T::size()) {
      //! Loads the values to vc-vector
      const auto coord = magic.load(n);

      //! Calculate the polarcoordinates
      const auto polarCoord = calculatePolarCoordinate(coord); //Ändern

      //! Store the values from the vc-vector
      magic.store(n, polarCoord);
    }

    for (size_t n = (inputSize - missingSize); n < inputSize; n++) {
      magic.setPolarCoordinate(
          n,
          calculatePolarCoordinate(magic.coordinate(n))); // Gibt Coordinate FLOAT
    }
  }

  state.SetItemsProcessed(state.iterations() * state.range_x());
  state.SetBytesProcessed(state.items_processed() * sizeof(typename T::value_type));
}

Vc_BENCHMARK_TEMPLATE(benchmarkGenericMemoryLayout,
                      outer_product<Vc_AVX_VECTORS, Vc_ALL_MEMORY_LAYOUT_TESTS>)
    ->Apply(dynamicAllCacheSize);
