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

    f32 const d1 = dot(O - r0, n);
    f32 const d2 = dot(-d, cross(O - r0, e2));
    f32 const d3 = dot(-d, cross(e1, O - r0));

    f32 const t  = d1 / d0;
    f32 const b1 = d2 / d0;
    f32 const b2 = d3 / d0;

    bool const hit = (1.f - b1 - b2) >= 0.f
                 &&         b1       >= 0.f
                 &&              b2  >= 0.f;

    return hit
         ? RayRange{t,     t}
         : RayRange{-1.f / 0.f, -1.f / 0.f}; 
}
