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

template <bool T> struct HasTwoOps { static constexpr bool hasTwoOps = T; };

#define Vc_MAKE_BASIC_ARITHMETIC_OPERATOR(name, operator)                                \
  struct name : public HasTwoOps<true> {                                                 \
    template <typename T> inline static T calculate(const T &x, const T &y) {            \
      return x operator y;                                                               \
    }                                                                                    \
  }

#define Vc_MAKE_ONE_OP_OPERATOR(name, operator)                                          \
  struct name : public HasTwoOps<false> {                                                \
    template <typename T> inline static T calculate(const T &x) { return operator(x); }  \
  }

#define Vc_MAKE_TWO_OP_OPERATOR(name, operator)                                          \
  struct name : public HasTwoOps<true> {                                                 \
    template <typename T> inline static T calculate(const T &x, const T &y) {            \
      return operator(x, y);                                                             \
    }                                                                                    \
  }

Vc_MAKE_BASIC_ARITHMETIC_OPERATOR(Add, +);
Vc_MAKE_BASIC_ARITHMETIC_OPERATOR(Sub, -);
Vc_MAKE_BASIC_ARITHMETIC_OPERATOR(Mul, *);
Vc_MAKE_BASIC_ARITHMETIC_OPERATOR(Div, / );
Vc_MAKE_BASIC_ARITHMETIC_OPERATOR(Mod, % );
Vc_MAKE_ONE_OP_OPERATOR(Sqrt, Vc::sqrt);
Vc_MAKE_ONE_OP_OPERATOR(Rsqrt, Vc::rsqrt);
Vc_MAKE_ONE_OP_OPERATOR(Reciprocal, Vc::reciprocal);
Vc_MAKE_ONE_OP_OPERATOR(Abs, Vc::abs);
Vc_MAKE_ONE_OP_OPERATOR(Round, Vc::round);
Vc_MAKE_ONE_OP_OPERATOR(Log, Vc::log);
Vc_MAKE_ONE_OP_OPERATOR(Log2, Vc::log2);
Vc_MAKE_ONE_OP_OPERATOR(Log10, Vc::log10);
Vc_MAKE_ONE_OP_OPERATOR(Exp, Vc::exp);
Vc_MAKE_ONE_OP_OPERATOR(Asin, Vc::asin);
Vc_MAKE_ONE_OP_OPERATOR(Atan, Vc::atan);
Vc_MAKE_TWO_OP_OPERATOR(Atan2, Vc::atan2);
Vc_MAKE_TWO_OP_OPERATOR(Min, Vc::min);
Vc_MAKE_TWO_OP_OPERATOR(Max, Vc::max);

template <typename T, typename P>
inline typename std::enable_if<P::hasTwoOps, T>::type calculate(T &x, T &y)
{
  return P::template calculate<T>(x, y);
}

template <typename T, typename P>
inline typename std::enable_if<!P::hasTwoOps, T>::type calculate(T &x, T &y)
{
  return P::template calculate<T>(x);
}

template <typename TT> void oneOp(benchmark::State &state) {
  using T = typename TT::template at<0>;
  using P = typename TT::template at<1>;

  T x = 1;
  T y = 1;
  T z;

  state.Run([&] {
    fakeRegisterModification(x);
    fakeRegisterModification(y);
    z = calculate<T, P>(x, y);
    fakeRegisterRead(z);
  });
  state.SetItemsProcessed(state.iterations() * T::size());
}

Vc_BENCHMARK_TEMPLATE(
    oneOp, concat<outer_product<Vc_ALL_VECTORS, Typelist<Add, Sub, Mul, Div>>,
                  outer_product<Vc_ALL_INT_VECTORS, Typelist<Mod>>,
                  outer_product<Vc_ALL_FLOAT_VECTORS,
                                Typelist<Sqrt, Rsqrt, Abs, Round, Log, Log2,
                                         Log10, Exp, Asin, Atan, Atan2, Min, Max>>>);
