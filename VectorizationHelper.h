#pragma once
#include <Vc/Vc>
#include <math.h>
#include <tuple>

using Vc::float_v;
using Vc::sqrt;
using Vc::atan2;

//!Template for calculation of polarcoordinates
template<typename T>
inline std::tuple<T, T> calcularePolarCoordinate(const T &x, const T &y)
{
    return std::make_tuple(sqrt(((x*x) + (y*y))), atan2(y, x) * 57.295780181884765625f);
}

//!Calculates the needed vector chunks for a given size
constexpr size_t numberOfChunks(size_t inputSize, size_t chunkSize)
{
    return (inputSize + chunkSize - 1) / chunkSize;
}
