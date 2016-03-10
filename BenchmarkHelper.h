#pragma once
#include "benchmark/benchmark_api.h"
#include <string>

inline void escape(void *p)
{
    asm volatile("" : : "g"(p) : "memory");
}

inline void clobber()
{
    asm volatile("" : : : "memory");
}

//!Generates a correct string for the plotter
const std::string getLabelString(const char *functionTitle, int itemCount, size_t itemSize)
{
    return std::string(functionTitle) + std::to_string(itemCount * itemSize);
}
