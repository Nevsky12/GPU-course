#pragma once
#include "squares.h"
#include <utils/types.h>
#include <gltf/utils/orthonormal.h>

#include <algorithm>
#include <numbers>

// pdfW relates probability measure mu to projected solid angle measure W: pdfW(w) = [dmu / dW] (w)
// pdfA relates mu to surface area measure A: pdfA(x) = [dmu / dA] (x)

// conversion coefficient from pdfW to pdfA: pdfA = pdfW * pdfWtoA
inline f32 pdfWtoA(vec3 const x1, vec3 const n1, vec3 const x2, vec3 const n2) noexcept
{
    return gltf::abs(dot(n1, x2 - x1) * dot(x1 - x2, n2)) / (dot(x2 - x1, x2 - x1) * dot(x1 - x2, x1 - x2));
}

inline vec3 uniformHemisphereSample(vec3 const norm) noexcept
    // pdfW = cosTheta / (2 * pi)
{
    f32 const cosTheta = generateUniformFloat();
    f32 const sinTheta = std::sqrt(1.f - cosTheta * cosTheta);

    f32 const phi = 2.f * std::numbers::pi_v<f32> * generateUniformFloat();
    return gltf::utils::orthonormal(norm) * vec3
    {
        sinTheta * std::cos(phi),
        sinTheta * std::sin(phi),
        cosTheta,
    };
}

inline vec3 uniformProjectedHemisphereSample(vec3 const norm) noexcept
    // pdfW = 1 / pi
{
    for(;;)
    {
        f32 const x = -1.f + 2.f * generateUniformFloat();
        f32 const y = -1.f + 2.f * generateUniformFloat();
        f32 const z2 = 1.f - x * x - y * y;
        if(z2 <= 0.f)
            continue;
        return gltf::utils::orthonormal(norm) * vec3{x, y, std::sqrt(z2)};
    }
}

inline vec3 uniformTrianglePoint(gltf::gvec<vec3, 3> const &triangle)
    // pdfA = 1 / A
{
    auto const [r0, r1, r2] = triangle;
    for(;;)
    {
        f32 const p = generateUniformFloat();
        f32 const q = generateUniformFloat();
        if(1.f - p - q < 0.f)
            continue;
        return (1.f - p - q) * r0
                    + p      * r1
                        + q  * r2;
    }
}

struct IndexSample
{
    u32 i;
    f32 probability;
};
template<std::ranges::range R>
auto indexSamplerFrom(R &&weights) noexcept
{
    std::vector<f32> w = {0.f};
    f64 sum = 0.;
    for(f64 const y : weights)
    {
        sum += y;
        w.push_back(f32(sum));
    }
    return [c = f32(1. / sum), weight = std::move(w)]() noexcept
    {
        auto const it = std::ranges::upper_bound(weight, generateUniformFloat() * weight.back());
        u32 const i = u32(it - weight.begin()) - 1u;
        return IndexSample{i, c * (weight[i + 1] - weight[i])};
    };
}
