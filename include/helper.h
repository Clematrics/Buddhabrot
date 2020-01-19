#pragma once

#include <utility>

template<class T>
std::pair<const T, const T> minmax(const T a, const T b)
{
    return (b < a) ? std::pair<const T, const T>(b, a)
                   : std::pair<const T, const T>(a, b);
}