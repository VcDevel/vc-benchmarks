/*
Copyright © 2016-2018 Matthias Kretz <kretz@kde.org>
Copyright © 2016 Björn Gaier

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
#include <memory>
#include "typetostring.h"

struct TemplateWrapper {
  std::vector<benchmark::internal::Benchmark *> benchmarks;

  void append(benchmark::internal::Benchmark *ptr) { benchmarks.push_back(ptr); }

  TemplateWrapper *operator->() { return this; }

  TemplateWrapper &Arg(int x) {
    for (auto &p : benchmarks) {
      p->Arg(x);
    }

    return *this;
  }

  TemplateWrapper &Range(int start, int limit) {
    for (auto &p : benchmarks) {
      p->Range(start, limit);
    }

    return *this;
  }

  TemplateWrapper &DenseRange(int start, int limit) {
    for (auto &p : benchmarks) {
      p->DenseRange(start, limit);
    }

    return *this;
  }

  TemplateWrapper &ArgPair(int x, int y) {
    for (auto &p : benchmarks) {
      p->ArgPair(x, y);
    }

    return *this;
  }

  TemplateWrapper &RangePair(int lo1, int hi1, int lo2, int hi2) {
    for (auto &p : benchmarks) {
      p->RangePair(lo1, hi1, lo2, hi2);
    }

    return *this;
  }

  template <typename... Ts> TemplateWrapper &Apply(Ts &&... args) {
    for (auto &p : benchmarks) {
      p->Apply(args...);
    }

    return *this;
  }

  TemplateWrapper &MinTime(double t) {
    for (auto &p : benchmarks) {
      p->MinTime(t);
    }

    return *this;
  }

  TemplateWrapper &UseRealTime() {
    for (auto &p : benchmarks) {
      p->UseRealTime();
    }

    return *this;
  }

  TemplateWrapper &Threads(int t) {
    for (auto &p : benchmarks) {
      p->Threads(t);
    }

    return *this;
  }

  TemplateWrapper &ThreadRange(int min_threads, int max_threads) {
    for (auto &p : benchmarks) {
      p->ThreadRange(min_threads, max_threads);
    }

    return *this;
  }

  TemplateWrapper &ThreadPerCpu() {
    for (auto &p : benchmarks) {
      p->ThreadPerCpu();
    }

    return *this;
  }

  operator int() { return 0; }
};

#define Vc_BENCHMARK_TEMPLATE(n_, ...)                                                   \
  template <unsigned int N>                                                              \
  TemplateWrapper BENCHMARK_PRIVATE_CONCAT(typeListFunc, n_, __LINE__)() {               \
    using TArg = __VA_ARGS__::at<N>;                                                     \
    constexpr auto *fptr = n_<TArg>;                                                     \
    static std::string name = #n_"<" + typeToString<TArg>() + '>';                       \
    TemplateWrapper wrapper =                                                            \
        BENCHMARK_PRIVATE_CONCAT(typeListFunc, n_, __LINE__)<N - 1>();                   \
    wrapper.append(benchmark::RegisterBenchmark(name.c_str(), fptr));                    \
    return wrapper;                                                                      \
  }                                                                                      \
                                                                                         \
  template <>                                                                            \
  TemplateWrapper BENCHMARK_PRIVATE_CONCAT(typeListFunc, n_, __LINE__)<0u>() {           \
    using TArg = __VA_ARGS__::at<0>;                                                     \
    constexpr auto *fptr = n_<TArg>;                                                     \
    static std::string name = #n_"<" + typeToString<TArg>() + '>';                       \
    TemplateWrapper wrapper;                                                             \
    wrapper.append(benchmark::RegisterBenchmark(name.c_str(), fptr));                    \
    return wrapper;                                                                      \
  }                                                                                      \
  int BENCHMARK_PRIVATE_CONCAT(variable, n_, __LINE__) =                                 \
      BENCHMARK_PRIVATE_CONCAT(typeListFunc, n_, __LINE__)<__VA_ARGS__::size() - 1>()

///////////////////////////////////////////////////////////////////////////////
// element_count<T>
template <class T, class = void>
struct element_count : std::integral_constant<std::size_t, 1> {};
template <class T>
struct element_count<T, decltype((void)T::size())>
    : std::integral_constant<std::size_t, T::size()> {};

///////////////////////////////////////////////////////////////////////////////
// fake_modification(x)
template <class T>
typename std::enable_if<(sizeof(T) < 16 && !std::is_class<T>::value)>::type
fake_modification(T &x) {
  asm("" :"+r"(x));
}
template <class T>
typename std::enable_if<(sizeof(T) >= 16 && !std::is_class<T>::value)>::type
fake_modification(T &x) {
  asm("" :"+x"(x));
}
template <class T, class A> void fake_modification(Vc::Vector<T, A> &x) {
  fake_modification(x.data());
}
template <class T, int N>
void fake_modification(Vc::Vector<T, Vc::simd_abi::fixed_size<N>> &x);
template <class T, std::size_t N, class V>
void fake_modification(Vc::SimdArray<T, N, V, N> &x) {
  fake_modification(internal_data(x));
}
template <class T, std::size_t N, class V, std::size_t Wt>
void fake_modification(Vc::SimdArray<T, N, V, Wt> &x) {
  fake_modification(internal_data0(x));
  fake_modification(internal_data1(x));
}
template <class T, int N>
void fake_modification(Vc::Vector<T, Vc::simd_abi::fixed_size<N>> &x) {
  fake_modification(static_cast<Vc::SimdArray<T, N> &>(x));
}

///////////////////////////////////////////////////////////////////////////////
// do_not_optimize(expr)
template <class T>
typename std::enable_if<(sizeof(T) < 16 && !std::is_class<T>::value)>::type
do_not_optimize(const T &x) {
  asm("" ::"r"(x));
}
template <class T>
typename std::enable_if<(sizeof(T) >= 16 && !std::is_class<T>::value)>::type
do_not_optimize(const T &x) {
  asm("" ::"x"(x));
}
template <class T, class A> void do_not_optimize(const Vc::Vector<T, A> &x) {
  do_not_optimize(x.data());
}
template <class T, int N>
void do_not_optimize(const Vc::Vector<T, Vc::simd_abi::fixed_size<N>> &x);
template <class T, std::size_t N, class V>
void do_not_optimize(const Vc::SimdArray<T, N, V, N> &x) {
  do_not_optimize(internal_data(x));
}
template <class T, std::size_t N, class V, std::size_t Wt>
void do_not_optimize(const Vc::SimdArray<T, N, V, Wt> &x) {
  do_not_optimize(internal_data0(x));
  do_not_optimize(internal_data1(x));
}
template <class T, int N>
void do_not_optimize(const Vc::Vector<T, Vc::simd_abi::fixed_size<N>> &x) {
  do_not_optimize(static_cast<const Vc::SimdArray<T, N> &>(x));
}
template <class T, class U> void do_not_optimize(const std::pair<T, U> &x) {
  do_not_optimize(x.first);
  do_not_optimize(x.second);
}

BENCHMARK_MAIN();
#endif // BENCHMARK_H
