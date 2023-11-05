#pragma once
#include "squares.h"
#include <utils/types.h>
#include <numbers>

struct IndexSample
{
    u32 i;
    f32 probability;
};

template<std::ranges::range R>
auto indexSamplerFrom(R &&weights) noexcept
{
    std::vector<f32> w = {0.f};
    for(auto const y : weights)
        w.push_back(y + w.back());
    return [c = 1.f / w.back(), weight = std::move(w)]() noexcept
    {
        auto const it = std::ranges::upper_bound
        (   
            weight,
            generate01N<1>()[0] * weight.back()
        );
        u32 const i = u32(it - weight.begin()) - 1u;
        return IndexSample{i, c * (weight[i + 1] - weight[i])};
    };
}

