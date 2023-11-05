#pragma once
#include <gltf/utils/aabb.h>

namespace gltf::utils
{

// (box[0], box[1]) -> ( (0, 0, 0), (1, 1, 1) )
inline constexpr mat4x3 toBoxSpace(AABB const &box) noexcept
{
    auto const unzero = lambdaR1(x, abs(x) == 0.f ? 1.f : x);
    vec3 const s = 0.99999988f / liftA1(unzero, box[1] - box[0]);
    return {fromDiagonal(s), -s * box[0]};
}
// ( (0, 0, 0), (1, 1, 1) ) -> (box[0], box[1])
inline constexpr mat4x3 fromBoxSpace(AABB const &box) noexcept
{
    return {1.00000012f * fromDiagonal(box[1] - box[0]), box[0]};
}

inline constexpr f32 sign(f32 x) noexcept {return x >= 0.f ? 1.f : -1.f;}
inline constexpr vec2 toOct(vec3 const &v) noexcept
    // unit sphere -> [-1; 1]^2
{
    vec3 const a = abs(v);

    vec2 const p = vec2(v) / (a.x + a.y + a.z);
    vec2 const r = liftA1(sign, vec2(v)) * (vec2(1.f) - abs(vec2(p.y, p.x)));

    return min(vec2(1.f), max(vec2(-1.f), v.z >= 0.f ? p : r));
}
inline vec3 fromOct(vec2 const &o) noexcept
    // [-1; 1]^2 -> unit sphere
{
    vec2 const a = abs(o);
    f32 const z = 1.f - a.x - a.y;

    vec2 const r = liftA1(sign, o) * (vec2(1.f) - vec2(a.y, a.x));

    vec3 const v = vec3((z >= 0.f ? o : r), z);
    return normalize(v);
}

template<typename T, u32 bits>
T toUnorm(f32 const x) noexcept
{
    u32 const maxU = (1u << bits) - 1u;
    assert(x >= 0.f && x <= 1.f);
    return T(std::round(x * maxU));
}
template<u32 bits>
f32 fromUnorm(u32 const u) noexcept
{
    u32 const maxU = (1u << bits) - 1u;
    assert(u <= maxU);
    return f32(u) / f32(maxU);
}

template<typename T, i32 bits>
T toSnorm(f32 const x) noexcept
{
    constexpr i32 maxI = (1 << (bits - 1)) - 1;
    assert(x >= -1.f && x <= 1.f);
    return T(std::round(x * maxI));
}
template<i32 bits>
f32 fromSnorm(i32 const i) noexcept
{
    constexpr i32 maxI = (1 << (bits - 1)) - 1;
    assert(i >= -maxI && i <= maxI);
    return f32(i) / f32(maxI);
}

inline u32 encodeU16x2(vec2 const &v) noexcept
{
    uvec2 const u = liftA1(toUnorm<u32, 16>, v);
    return (u.y << 16u) | u.x;
}
inline vec2 decodeU16x2(u32 const n) noexcept
{
    uvec2 const u = {n & 0xffffu, n >> 16u};
    return liftA1(fromUnorm<16>, u);
}

inline i32 encodeS16x2(vec2 const &v) noexcept
{
    ivec2 const i = liftA1(toSnorm<i32, 16>, v);
    return (i.y << 16) | (i.x & 0xffff);
}
inline vec2 decodeS16x2(i32 const n) noexcept
{
    ivec2 const i = {(n << 16) >> 16, n >> 16};
    return liftA1(fromSnorm<16>, i);
}

inline uvec2 encodeU21x3(vec3 const &v) noexcept
{
    uvec3 const u = liftA1(toUnorm<u32, 21>, v);
    auto const [hi, lo] = uvec2{(u.y >> 11u) << 21u, u.y << 21u};
    return
    {
        hi | u.x,
        lo | u.z,
    };
}
inline vec3 decodeU21x3(uvec2 const u) noexcept
{
    return
    {
        f32(  u.x & 0x001fffffu) / f32(0x001fffffu),
        f32(((u.x & 0x7fe00000u) >> 10u) | (u.y >> 21u)) / f32(0x001fffffu),
        f32(  u.y & 0x001fffffu) / f32(0x001fffffu),
    };
}

inline f32 decodeSRGB(u8 const u) noexcept
{
    f32 const x = fromUnorm<8>(u);
    f32 const f = std::pow((x + 0.055f) / 1.055f, 2.4f);
    return f > 0.04045f ? f : x / 12.92f;
}
inline u8 encodeSRGB(f32 const x) noexcept
{
    f32 const f = x < 0.0031308f
        ? x * 12.92f
        : 1.055f * std::pow(x, 1.f / 2.4f) - 0.055f;
    return toUnorm<u8, 8>(f);
}

} // namespace gltf::utils
