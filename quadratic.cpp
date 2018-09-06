/*{{{
Copyright © 2016 Guilherme Amadio
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

#include "benchmark.h"
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <malloc.h>
#include <x86intrin.h>
#include <Vc/Vc>

static const int N = (8 * 1024 * 1024);

// solve ax2 + bx + c = 0

template <typename T>
void QuadSolve(const T &a, const T &b, const T &c, T &x1, T &x2, int &roots)
{
  T a_inv = T(1.0) / a;
  T delta = b * b - T(4.0) * a * c;
  T s = (b >= 0) ? T(1.0) : T(-1.0);

  roots = delta > std::numeric_limits<T>::epsilon() ? 2 : delta < T(0.0) ? 0 : 1;

  switch (roots) {
  case 2:
    x1 = T(-0.5) * (b + s * std::sqrt(delta));
    x2 = c / x1;
    x1 *= a_inv;
    return;

  case 0:
    return;

  case 1:
    x1 = x2 = T(-0.5) * b * a_inv;
    return;

  default:
    return;
  }
}

#if defined(__AVX2__)

// explicit AVX2 code using intrinsics

void __attribute__((noinline))
QuadSolveAVX(const float *__restrict__ a, const float *__restrict__ b, const float *__restrict__ c,
             float *__restrict__ x1, float *__restrict__ x2, int *__restrict__ roots)
{
  __m256 one = _mm256_set1_ps(1.0f);
  __m256 va = _mm256_load_ps(a);
  __m256 vb = _mm256_load_ps(b);
  __m256 zero = _mm256_set1_ps(0.0f);
  __m256 a_inv = _mm256_div_ps(one, va);
  __m256 b2 = _mm256_mul_ps(vb, vb);
  __m256 eps = _mm256_set1_ps(std::numeric_limits<float>::epsilon());
  __m256 vc = _mm256_load_ps(c);
  __m256 negone = _mm256_set1_ps(-1.0f);
  __m256 ac = _mm256_mul_ps(va, vc);
  __m256 sign = _mm256_blendv_ps(negone, one, _mm256_cmp_ps(vb, zero, _CMP_GE_OS));
#if defined(__FMA__)
  __m256 delta = _mm256_fmadd_ps(_mm256_set1_ps(-4.0f), ac, b2);
  __m256 r1 = _mm256_fmadd_ps(sign, _mm256_sqrt_ps(delta), vb);
#else
  __m256 delta = _mm256_sub_ps(b2, __256_mul_ps(_mm256_set1_ps(-4.0f), ac));
  __m256 r1 = _mm256_add_ps(vb, _mm256_mul_ps(sign, _mm256_sqrt_ps(delta)));
#endif
  __m256 mask0 = _mm256_cmp_ps(delta, zero, _CMP_LT_OS);
  __m256 mask2 = _mm256_cmp_ps(delta, eps, _CMP_GE_OS);
  r1 = _mm256_mul_ps(_mm256_set1_ps(-0.5f), r1);
  __m256 r2 = _mm256_div_ps(vc, r1);
  r1 = _mm256_mul_ps(a_inv, r1);
  __m256 r3 = _mm256_mul_ps(_mm256_set1_ps(-0.5f), _mm256_mul_ps(vb, a_inv));
  __m256 nr = _mm256_blendv_ps(one, _mm256_set1_ps(2), mask2);
  nr = _mm256_blendv_ps(nr, _mm256_set1_ps(0), mask0);
  r3 = _mm256_blendv_ps(r3, zero, mask0);
  r1 = _mm256_blendv_ps(r3, r1, mask2);
  r2 = _mm256_blendv_ps(r3, r2, mask2);
  _mm256_store_si256((__m256i *)roots, _mm256_cvtps_epi32(nr));
  _mm256_store_ps(x1, r1);
  _mm256_store_ps(x2, r2);
}
#endif

// explicit SIMD code using Vc

using Float_v = Vc::Vector<float>;
using Int32_v = Vc::Vector<int32_t>;

void QuadSolveSIMD(Float_v const &a, Float_v const &b,
                   Float_v const &c, Float_v &x1, Float_v &x2,
                   Int32_v &roots)
{
  using FMask = typename Float_v::Mask;
  using IMask = typename Int32_v::Mask;

  Float_v a_inv = Float_v(1.0f) / a;
  Float_v delta = b * b - Float_v(4.0f) * a * c;
  Float_v sign = iif(FMask(b >= Float_v(0.0f)), Float_v(1.0f), Float_v(-1.0f));

  FMask mask0(delta < Float_v(0.0f));
  FMask mask2(delta >= Float_v(std::numeric_limits<float>::epsilon()));

  Float_v root1 = Float_v(-0.5f) * (b + sign * std::sqrt(delta));
  Float_v root2 = c / root1;
  root1 = root1 * a_inv;

  FMask mask1 = !(mask2 || mask0);

  x1(mask2) = root1;
  x2(mask2) = root2;
  roots = iif(simd_cast<IMask>(mask2), Int32_v(2), Int32_v(0));

  if (mask1.isEmpty())
    return;

  root1 = Float_v(-0.5f) * b * a_inv;
  roots(simd_cast<IMask>(mask1)) = Int32_v(1);
  x1(mask1) = root1;
  x2(mask1) = root1;
}

struct Data {
  Data() {
    srand48(time(NULL));
    for (int i = 0; i < N; i++) {
      a[i] = 10.0 * (drand48() - 0.5);
      b[i] = 10.0 * (drand48() - 0.5);
      c[i] = 50.0 * (drand48() - 0.5);
      x1[i] = 0.0;
      x2[i] = 0.0;
      roots[i] = 0;
    }
  }

  float *a = (float *)memalign(64, N * sizeof(float));
  float *b = (float *)memalign(64, N * sizeof(float));
  float *c = (float *)memalign(64, N * sizeof(float));

  int *roots = (int *)memalign(64, N * sizeof(int));
  float *x1 = (float *)memalign(64, N * sizeof(float));
  float *x2 = (float *)memalign(64, N * sizeof(float));
};

void scalar(benchmark::State &state) {
  Data d;
  for (auto _ : state) {
    for (int i = 0; i < N; i++) {
      QuadSolve<float>(d.a[i], d.b[i], d.c[i], d.x1[i], d.x2[i], d.roots[i]);
    }
  }
}

void intrinsics(benchmark::State &state) {
  Data d;
  for (auto _ : state) {
    for (int i = 0; i < N; i += 8) {
      QuadSolveAVX(&d.a[i], &d.b[i], &d.c[i], &d.x1[i], &d.x2[i], &d.roots[i]);
    }
  }
}

void TestQuadSolve(const float *__restrict__ a, const float *__restrict__ b,
                   const float *__restrict__ c, float *__restrict__ x1,
                   float *__restrict__ x2, int *__restrict__ roots) {
  for (size_t i = 0; i < N; i += Float_v::Size) {
    QuadSolveSIMD((Float_v &)(a[i]), (Float_v &)(b[i]), (Float_v &)(c[i]),
                  (Float_v &)(x1[i]), (Float_v &)(x2[i]), (Int32_v &)(roots[i]));
  }
}

void vc(benchmark::State &state) {
  Data d;
  for (auto _ : state) {
    TestQuadSolve(d.a, d.b, d.c, d.x1, d.x2, d.roots);
  }
}

BENCHMARK(scalar);
#ifdef __AVX2__
BENCHMARK(intrinsics);
#endif
BENCHMARK(vc);
