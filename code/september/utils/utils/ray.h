#pragma once
#include "vec3.h"

struct Ray
{
    vec3 origin;
    vec3 direction;
};

struct RayRange
{
    f32 tMin, tMax;
};
inline const RayRange zeroToInf = {0.f, 1.f / 0.f};

inline bool nonempty(RayRange const r) noexcept {return r.tMin <= r.tMax;}
inline bool    empty(RayRange const r) noexcept {return !nonempty(r);}
inline bool contains(RayRange const r, f32 const t) noexcept {return r.tMin <= t && t <= r.tMax;}

inline RayRange operator+(RayRange const r1, RayRange const r2) noexcept
{
    return
    {
        r1.tMin < r2.tMin ? r2.tMin : r1.tMin,
        r1.tMax < r2.tMax ? r1.tMax : r2.tMax,
    };
}

// if used as an intersection result:
inline f32 hitDistance(RayRange const r) noexcept {return r.tMin;}
