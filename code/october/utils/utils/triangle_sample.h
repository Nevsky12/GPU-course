#pragma once
#include "squares.h"
#include <gltf/utils/triangle.h>

struct TriangleSample
{
    vec3 r;
    vec3 n;
    f32 pdf;
};
inline TriangleSample uniformTrianglePoint(Triangle const &triangle) noexcept
{
    auto const [r0, r1, r2] = triangle;
    for(;;)
    {
        auto const [p, q] = generate01N<2>();
        if(1.f - p - q < 0.f)
            continue;
        vec3 const r = (1.f - p - q) * r0
                            + p      * r1
                                + q  * r2;
        vec3 const SN = cross(r1 - r0, r2 - r0);
        return 
        {
            .r = r,
            .n = normalize(SN),
            .pdf = 2.f / length(SN),
        };
    }
}

