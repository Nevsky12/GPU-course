#pragma once
#include "squares.h"
#include <gltf/utils/triangle.h>


struct TriangleSample
{
    vec3 r;
    f32 pdf;
};
inline TriangleSample uniformTrianglePoint( Triangle const &triangle
                                          , vec3 const rayOrigin
                                          , vec3 const rayNorm
                                          ) noexcept
{
    vec3 const o = rayOrigin;
    vec3 const N = rayNorm;
    auto const [r0, r1, r2] = triangle;
    for(;;)
    {
        auto const [p, q] = generate01N<2>();
        if(1.f - p - q < 0.f)
            continue;
        vec3 const r = (1.f - p - q) * r0
                            + p      * r1
                                + q  * r2;
        vec3 const dr = r - o;
        vec3 const Sn = 0.5f * gltf::cross(r1 - r0, r2 - r0);
        vec3 const n  = gltf::normalize(Sn);
        return
        {
            .r = r,
            .pdf =     dot(dr, dr) * dot(dr,  dr)
                 /
             gltf::abs(dot(N,  dr) * dot(n,  dr) * length(Sn)),
        };
    }
}

