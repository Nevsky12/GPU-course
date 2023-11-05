#pragma once
#include "squares.h"
#include <utils/types.h>
#include <numbers>

struct HemisphereSample
{
    vec3 direction;
    f32 pdf;
};

inline HemisphereSample uniformHemisphereSample() noexcept
{
    f32 const cosTheta = generateUniformFloat();
    f32 const sinTheta = std::sqrt(1.f - cosTheta * cosTheta);

    f32 const twopi = 2.f * std::numbers::pi_v<f32>;
    f32 const phi = twopi * generateUniformFloat();
    return
    {
        .direction =
        {
            sinTheta * std::cos(phi),
            sinTheta * std::sin(phi),
            cosTheta,
        },
        .pdf = 1.f / twopi,
    };
}

struct ProjectedHemisphereSample
{
    vec3 direction;
    f32 pdf;
};
inline HemisphereSample uniformProjectedHemisphereSample() noexcept
{
    for(;;)
    {
        f32 const x = -1.f + 2.f * generateUniformFloat();
        f32 const y = -1.f + 2.f * generateUniformFloat();
        if(x * x + y * y > 1.f)
            continue;
        return
        {
            .direction =
            {
                x,
                y,
                std::sqrt(1.f - x * x - y * y)
            },
            .pdf = 1.f / std::numbers::pi_v<f32>,
        };
    }
}
