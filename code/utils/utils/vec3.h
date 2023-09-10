#pragma once
#include "types.h"
#include <cmath>

struct vec3
{
    f32 x, y, z;

    f32       &operator[](u64 const i)       noexcept {return (&x)[i];}
    f32 const &operator[](u64 const i) const noexcept {return (&x)[i];}
};

inline vec3 operator+(vec3 const v1, vec3 const v2) noexcept
{
    return
    {
        .x = v1.x + v2.x,
        .y = v1.y + v2.y,
        .z = v1.z + v2.z,
    };
}
inline vec3 operator-(vec3 const v1, vec3 const v2) noexcept
{
    return
    {
        .x = v1.x - v2.x,
        .y = v1.y - v2.y,
        .z = v1.z - v2.z,
    };
}
inline vec3 operator-(vec3 const v) noexcept
{
    return
    {
        .x = -v.x,
        .y = -v.y,
        .z = -v.z,
    };
}
inline vec3 operator*(vec3 const v1, vec3 const v2) noexcept
{
    return
    {
        .x = v1.x * v2.x,
        .y = v1.y * v2.y,
        .z = v1.z * v2.z,
    };
}
inline vec3 operator*(vec3 const v, f32 const f) noexcept
{
    return
    {
        .x = v.x * f,
        .y = v.y * f,
        .z = v.z * f,
    };
}
inline vec3 operator/(vec3 const v1, vec3 const v2) noexcept
{
    return
    {
        .x = v1.x / v2.x,
        .y = v1.y / v2.y,
        .z = v1.z / v2.z,
    };
}
inline vec3 operator/(vec3 const v, f32 const f) noexcept
{
    return
    {
        .x = v.x / f,
        .y = v.y / f,
        .z = v.z / f,
    };
}
inline f32 dot(vec3 const v1, vec3 const v2) noexcept
{
    return v1.x * v2.x
         + v1.y * v2.y
         + v1.z * v2.z;
}
inline f32 length(vec3 const v) noexcept
{
    return std::sqrt(dot(v, v));
}
inline vec3 normalize(vec3 const v) noexcept
{
    return v / length(v);
}
inline vec3 cross(vec3 const v1, vec3 const v2) noexcept
{
    return
    {
        .x = v1.y * v2.z - v1.z * v2.y,
        .y = v1.z * v2.x - v1.x * v2.z,
        .z = v1.x * v2.y - v1.y * v2.x,
    };
}
