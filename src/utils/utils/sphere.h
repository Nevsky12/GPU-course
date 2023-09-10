#pragma once
#include "ray.h"

struct Sphere
{
    vec3 origin;
    f32 radius;
};

inline RayRange raySphereIntersection(Ray const ray, Sphere const sphere) noexcept
{
    vec3 const s = ray.origin - sphere.origin;
    f32 const ds = dot(ray.direction, s);
    f32 const d2 = dot(ray.direction, ray.direction);
    f32 const det = ds * ds + d2 * (sphere.radius * sphere.radius - dot(s, s));
    return
    {
        .tMin = (-ds - std::sqrt(det)) / d2,
        .tMax = (-ds + std::sqrt(det)) / d2,
    };
}
