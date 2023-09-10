#pragma once
#include "ray.h"

struct Triangle
{
    vec3 r0;
    vec3 r1;
    vec3 r2;
};

inline RayRange rayTriangleIntersection(Ray const ray, Triangle const triangle) noexcept
{
    auto const &[r0, r1, r2] = triangle;
    auto const &[O,       d] = ray;

    vec3 const e1 = r1 - r0;
    vec3 const e2 = r2 - r0;
    
    vec3 const n = cross(e1, e2);
    f32 const d0 = dot(-d, n);

    f32 const infNeg = -1.0f / 0.f;

    f32 const invD0 = f32(1) / d0;

    f32 const d1 = dot(O - r0, n);
    f32 const d2 = dot(-d, cross(O - r0, e2));
    f32 const d3 = dot(-d, cross(e1, O - r0));

    f32 const b1 = d2 * invD0;
    f32 const b2 = d3 * invD0;

    if (!(b1 >= f32(0)))
        return infNeg;
    if (!(b2 >= f32(0) && b1 + b2 <= f32(1)))
        return infNeg;

    f32 const t = d1 * invD0;
    return {t, t}; 
}
