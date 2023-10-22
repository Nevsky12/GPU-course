#pragma once
#include "ray.h"
#include <optional>
#include <iostream>

struct Triangle
{
    vec3 r0, r1, r2;
};

struct RayTriangleIntersection
{
    float t;
    vec2 barycentrics;
    bool frontFacing;
};

template<typename T>
auto interpolate(T const &r, vec2 const barycentrics) noexcept
{
    auto const [p, q] = barycentrics;
    return r[0] * (1.f - p - q)
         + r[1] *        p
         + r[2] *            q;
}

inline f32 hitDistance(RayTriangleIntersection const intersection) noexcept
{
    return intersection.t;
}

inline std::optional<RayTriangleIntersection> rayIntersection( const Ray ray
                                                             , const Triangle tri
                                                             , const RayRange range
                                                             ) noexcept
{
    auto const [r0, r1, r2] = tri;
    vec3 const a =  r1 - r0;
    vec3 const b =  r2 - r0;
    vec3 const c = ray.origin - r0;
    vec3 const d = -ray.direction;
    

    f32 const det0 = dot(d, cross(a, b));

//    std::cout << "det0: " << det0 << std::endl;

//    auto const [Dir, orig] = ray;
//    std::cout << "ray (orig, dir): " << " (" << orig.x << ", " << orig.y << ", " << orig.z << ")"
//                                      << " (" << Dir.x  << ", " << Dir.y  << ", " << Dir.z  << std::endl;
                                     

    f32 const t = dot(c, cross(a, b)) / det0;
    f32 const p = dot(d, cross(c, b)) / det0;
    f32 const q = dot(d, cross(a, c)) / det0;

//    std::cout << "p, q: " << p << ", " << q << std::endl; 
    return contains(range, t)
        && (1.f - p - q) >= 0.f
        &&        p      >= 0.f
        &&            q  >= 0.f
        ? std::optional{RayTriangleIntersection{t, vec2{p, q}, det0 > 0.f}}
        : std::nullopt;
}
