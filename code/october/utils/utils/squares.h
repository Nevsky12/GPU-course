#pragma once
#include <utils/types.h>
#include <bit>

// https://gitlab.com/Yielduck/squares
// https://squaresrng.wixsite.com/rand
inline constexpr u32 uniformU32(u64 const counter) noexcept
{
    u64 const key = 0x5489de2cbce65297ull;
    u64 const n0 = counter * key;
    u64 n = n0;

    for(u32 round = 0u; round < 4u; ++round)
    {
        n = (round & 0x1u) == 0u // if even
            ? n * n + n0
            : n * n + n0 + key;
        n = (n >> 32u) | (n << 32u);
    }

    return static_cast<u32>(n);
}
struct Squares
{
    u64 counter = 0;

    using result_type = u32;
    static constexpr result_type min() noexcept {return 0;}
    static constexpr result_type max() noexcept {return UINT32_MAX;}

    constexpr result_type operator()() noexcept
    {
        return uniformU32(counter++);
    }

    constexpr Squares split() noexcept
    {
        u64 const lower = this->operator()();
        u64 const upper = this->operator()();
        return {(upper << 32) | lower};
    }
};

inline constexpr f32 uniformFloat(u32 const u) noexcept // -> [0.f, 1.f)
{
    f32 const f = std::bit_cast<f32>(std::bit_cast<u32>(0.5f) | (u & 0x007fffffu)); // [0.5f, 1.f)
    return ((u & 0x00800000u) == 0u) ? f - 0.5f : f;
}

f32 generateUniformFloat() noexcept
{
    static thread_local Squares generator = {.counter = u64(rand())};
    return uniformFloat(generator());
}

template<u32 N>
constexpr std::array<f32, N> generate01N() noexcept // -> [0.f, 1.f)^N
{
    return []<u32... i>(std::integer_sequence<u32, i...>) noexcept
    {
        static thread_local u64 counter = 0;
        return std::array
        {
            uniformFloat(uniformU32(++counter + (i - i)))...
        };
    }(std::make_integer_sequence<u32, N>{});
}
