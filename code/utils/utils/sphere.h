#pragma once
#include "ray.h"
#include <optional>

struct Sphere
{
    vec3 origin;
    f32 radius;
};

inline std::optional<RayRange> rayIntersection( Ray const ray
                                              , Sphere const sphere
                                              , RayRange const range
                                              ) noexcept
{
    vec3 const s = ray.origin - sphere.origin;
    f32 const ds = dot(ray.direction, s);
    f32 const d2 = dot(ray.direction, ray.direction);
    f32 const det = ds * ds + d2 * (sphere.radius * sphere.radius - dot(s, s));
    RayRange const r =
    {
        .tMin = (-ds - std::sqrt(det)) / d2,
        .tMax = (-ds + std::sqrt(det)) / d2,
    };
    return nonempty(range + r)
        ? std::optional{range + r}
        : std::nullopt;
}
