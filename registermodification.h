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
inline void
fakeRegisterModification(Vc::Vector<T, Vc::VectorAbi::Scalar> &modifiedValue) {
  asm volatile("" : "+r"(modifiedValue));
}

template <typename T>
inline typename std::enable_if<std::is_arithmetic<T>::value>::type
fakeRegisterModification(T &modifiedValue) {
  asm volatile("" : "+r"(modifiedValue));
}

template <typename T> inline void fakeMemoryRead(T &readedValue) {
  asm volatile("" ::"m"(readedValue));
}

template <typename T, typename B>
inline void fakeRegisterRead(Vc::Vector<T, B> &readedValue) {
  asm volatile("" ::"x"(readedValue));
}

template <typename T>
inline void fakeRegisterRead(Vc::Vector<T, Vc::VectorAbi::Scalar> &readedValue) {
  asm volatile("" ::"r"(readedValue));
}

template <typename T>
inline typename std::enable_if<std::is_arithmetic<T>::value>::type
fakeRegisterRead(T &readedValue) {
  asm volatile("" ::"r"(readedValue));
}

#endif // REGISTER_MODIFICATION_H
