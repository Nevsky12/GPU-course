#pragma once
#include <cassert>
#include <chrono>
#include <ranges>

namespace utils
{

template<unsigned int M, typename F>
double measureExecutionTime(F &&func) noexcept
{
    auto const t0 = std::chrono::high_resolution_clock::now();
    for(unsigned int m = 0u; m < M; ++m)
        func();
    auto const t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> const dt = t1 - t0;
    return dt.count() / double(M);
}

template<std::ranges::range R>
auto stats(R &&range) noexcept
{
    using T = std::ranges::range_value_t<R>;
    T s1 = T(0);
    T s2 = T(0);

    unsigned int n = 0u;
    for(T const &x : range)
    {
        s1 +=     x;
        s2 += x * x;
        ++n;
    }
    assert(n > 1u);
    T const mean = s1 / T(n);
    T const sqr  = s2 / T(n);
    return std::pair<T, T>{mean, (sqr - mean * mean) / T(n - 1u)};
}

template<unsigned int N, unsigned int M = 1, typename F>
auto stats(F &&func) noexcept
{
    double execTime[N];
    for(double &t : execTime)
        t = measureExecutionTime<M>(static_cast<F &&>(func));
    return stats(execTime);
}

} // namespace utils

