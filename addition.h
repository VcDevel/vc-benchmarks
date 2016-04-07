#ifndef ADDITION_H
#define ADDITION_H

//! Addition of two vectors
template <typename T> void additionVectorVector(benchmark::State &state) {
  T x(T::Random());
  T y(T::Random());
  T z;

  while (state.KeepRunning()) {
    fakeMemoryModification(x);
    fakeMemoryModification(y);
    z = x + y;
    fakeRegisterRead(z);
  }

  state.SetItemsProcessed(T::size());

#ifdef USE_LOG
  std::clog << "Finnished: AdditionVecVec\n";
#endif
}
#endif // ADDITION_H
