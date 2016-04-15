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
#ifndef ADDITION_H
#define ADDITION_H

template<bool T>
struct HasTwoOps {
    static constexpr bool hasTwoOps = T;
};

struct Addition : public HasTwoOps<true> {
    template <typename T>
    inline static T calculate(const T &x, const T &y) {
        return x + y;
    }
};

struct Substraction : public HasTwoOps<true> {
    template <typename T>
    inline static T calculate(const T &x, const T &y) {
        return x - y;
    }
};

struct Multiplication : public HasTwoOps<true> {
    template<typename T>
    inline static T calculate(const T &x, const T &y) {
        return x * y;
    }
};

struct Division : public HasTwoOps<true> {
    template<typename T>
    inline static T calculate(const T &x, const T &y) {
        return x / y;
    }
};

struct Modulo : public HasTwoOps<true> {
    template<typename T>
    inline static typename std::enable_if<std::is_integral<typename T::value_type>::value , T>::type calculate(const T &x, const T &y) {
        return x%y;
    }

    template<typename T>
    inline static typename std::enable_if<std::is_floating_point<typename T::value_type>::value , T>::type calculate(const T &x, const T &y) {
        T z;

        for(size_t n = 0; n < T::size(); n++) {
            z[n] = (typename T::value_type)(std::fmod(x[n], y[n]));
        }

        return z;
    }
};

struct Sinus : public HasTwoOps<false> {
    template<typename T>
    inline static T calculate(const T &x) {
        return Vc::sin(x);
    }
};

struct Cosinus : public HasTwoOps<false> {
    template<typename T>
    inline static T calculate(const T &x) {
        return Vc::cos(x);
    }
};

template<typename T, typename P>
typename std::enable_if<P::hasTwoOps, T>::type calculate(T &x, T &y) {
    return P::template calculate<T>(x, y);
}

template<typename T, typename P>
typename std::enable_if<!P::hasTwoOps, T>::type calculate(T &x, T &y) {
    return P::template calculate<T>(x);
}

//! Addition of two vectors
template <typename TT>
void oneOp(benchmark::State &state) {
  using T = typename TT::template at<0>;
  using P = typename TT::template at<1>;

  T x(T::Random());
  T y(T::Random());
  T z;

  while (state.KeepRunning()) {
    fakeMemoryModification(x);
    fakeMemoryModification(y);

    z = calculate<T, P>(x, y);

    fakeRegisterRead(z);
  }

  state.SetItemsProcessed(T::size());
}
#endif // ADDITION_H
