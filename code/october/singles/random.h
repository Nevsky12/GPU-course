#include <cassert>
#include <cmath>
#include <cstdint>
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using f32 = float;
using f64 = double;

#include <array>
#include <bit>

inline constexpr f32 uniformFloat(u32 const bits) noexcept // -> [0.f, 1.f)
{
    f32 const f = std::bit_cast<f32>(std::bit_cast<u32>(0.5f)
                | (bits & 0x007fffffu)); // [0.5f, 1.f)
    return ((bits & 0x00800000u) == 0u) ? f - 0.5f : f;
}
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
