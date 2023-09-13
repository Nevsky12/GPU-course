#pragma once
#include "ray.h"

struct AABB
{
     vec3 rMin;
     vec3 rMax;
};

inline RayRange rayAABBIntersection(Ray const ray, AABB const aabb) noexcept
{
    vec3 const tMin = (aabb.rMin - ray.origin) / ray.direction;
    vec3 const tMax = (aabb.rMax - ray.origin) / ray.direction;

    auto const min = [](f32 const a, f32 const b) noexcept {return a < b ? a : b;};
    auto const max = [](f32 const a, f32 const b) noexcept {return a > b ? a : b;};

    vec3 const t1 = {min(tMin.x, tMax.x), min(tMin.y, tMax.y), min(tMin.z, tMax.z)};
    vec3 const t2 = {max(tMin.x, tMax.x), max(tMin.y, tMax.y), max(tMin.z, tMax.z)};

    f32 const tNear = max(max(t1.x, t1.y), t1.z);
    f32 const tFar  = min(min(t2.x, t2.y), t2.z); 
    return
    {
        .tMin = tNear,
        .tMax = tFar,
    };
}
