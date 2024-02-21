#pragma once
#include "simd_ray.h"

struct Camera
{
    vec3 origin, at, up;
    f32 fov, aspectRatio;

    Ray<vec3> castRay(f32 const u, f32 const v) const noexcept
    {
        vec3 const z = normalize(origin - at);
        vec3 const x = normalize(cross(up, z));
        vec3 const y = cross(z, x);

        vec3 const d = x * (u * std::tan(fov) * aspectRatio)
                     + y * (v * std::tan(fov)              )
                     - z;
        return
        {
            .pos = origin,
            .dir = normalize(d),
        };
    }
};
