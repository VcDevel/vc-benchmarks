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
#ifndef REGISTER_MODIFICATION_H
#define REGISTER_MODIFICATION_H
#include <Vc/Vc>

inline void escape(void *p) { asm volatile("" : : "g"(p) : "memory"); }

inline void clobber() { asm volatile("" : : : "memory"); }

template <typename T> inline void fakeMemoryModification(T &modifiedValue) {
  asm volatile("" : "+m"(modifiedValue));
}

template <typename T, typename B>
inline void fakeRegisterModification(Vc::Vector<T, B> &modifiedValue) {
  asm volatile("" : "+x"(modifiedValue));
}

template <typename T>
inline typename std::enable_if<std::is_integral<T>::value>::type
fakeRegisterModification(T &modifiedValue) {
  asm volatile("" : "+r"(modifiedValue));
}

template <typename T>
inline typename std::enable_if<std::is_floating_point<T>::value>::type
fakeRegisterModification(T &modifiedValue)
{
  asm volatile("" : "+x"(modifiedValue));
}

template <typename T>
inline void fakeRegisterModification(Vc::Vector<T, Vc::VectorAbi::Scalar> &x)
{
  fakeRegisterModification(x.data());
}

template <typename T> inline void fakeMemoryRead(T &readedValue) {
  asm volatile("" ::"m"(readedValue));
}

template <typename T, typename B>
inline void fakeRegisterRead(Vc::Vector<T, B> &readedValue) {
  asm volatile("" ::"x"(readedValue));
}

template <typename T>
inline typename std::enable_if<std::is_integral<T>::value>::type
fakeRegisterRead(T &readedValue) {
  asm volatile("" ::"r"(readedValue));
}

template <typename T>
inline typename std::enable_if<std::is_floating_point<T>::value>::type
fakeRegisterRead(T &readedValue) {
  asm volatile("" ::"x"(readedValue));
}

template <typename T>
inline void fakeRegisterRead(Vc::Vector<T, Vc::VectorAbi::Scalar> &x) {
  fakeRegisterRead(x.data());
}

#endif // REGISTER_MODIFICATION_H
