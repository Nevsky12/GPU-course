#pragma once
#include "ray.h"

struct Triangle
{
    vec3 r0;
    vec3 r1;
    vec3 r2;
};

inline f32 rayTriangleIntersection(Ray const ray, Triangle const triangle) noexcept
{
    auto const &[r0, r1, r2] = triangle;
    auto const &[O,       d] = ray;

    vec3 const e1 = r1 - r0;
    vec3 const e2 = r2 - r0;
    
    vec3 const n = cross(e1, e2);
    f32 const d0 = dot(-d, n);

    f32 const infNeg = -1.0f / 0.f;
    if (d0 < 1e-6f)
        return infNeg;

    f32 const invD0 = f32(1) / d0;

    f32 const d1 = dot(O - r0, n);
    f32 const d2 = dot(-d, cross(O - r0, e2));
    f32 const d3 = dot(-d, cross(e1, O - r0));

    f32 const b1 = d2 * invD0;
    f32 const b2 = d3 * invD0;

    if (b1 < f32(0) || b1      > f32(1))
        return infNeg;
    if (b2 < f32(0) || b1 + b2 > f32(1))
        return infNeg;

    return d1 * invD0;
    
    /*
    vec3 const pvec = cross(d, e2);
    f32 const d0 = dot(e1, pvec);

    if (d0 < 0.000001f)
        return -1.0f / 0.f;

    f32 const invD0 = f32(1) / d0;

    vec3 const tvec = ray.origin - r0;
    f32 const b1 = dot(tvec, pvec) * invD0;

    if (b1 < 0 || b1 > 1)
        return -1.0f / 0.f;

    vec3 const qvec = cross(tvec, e1);
    f32 const b2 = dot(ray.direction, qvec) * invD0;

    if (b2 < 0 || b1 + b2 > 1)
        return -1.0f / 0.f;

    return dot(e2, qvec) * invD0;
    */
}
