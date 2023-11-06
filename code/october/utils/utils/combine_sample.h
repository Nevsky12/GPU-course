#pragma once
#include "index_sample.h"
#include "triangle_sample.h"

struct EmissiveTriangleInfo
{
    gltf::gvec<vec3, 3> pos;
    vec3 emission;
    u32 instanceI;
    u32 triangleI;
};

struct SourceSample
{
    vec3 dr;
    vec3 n;
    f32 pdf;
};

f32 triangleWeight(EmissiveTriangleInfo const &info) noexcept
{
    auto const [r0, r1, r2] = info.pos;
    return length(cross(r2 - r0, r1 - r0)) * length(info.emission);
}

template<std::ranges::range R, typename DF, typename Weighter>
requires( requires( std::ranges::range_value_t<R> const r
                  , Weighter const wr
                  , DF const df
                  ) 
              {
                  {r.pos} -> std::convertible_to<vec3>;
                  {wr(r)} -> std::convertible_to<f32>;
                  {df(r.pos)};
              }
        )
auto sourcesSamplerFrom(R const &sources, DF const &df, Weighter const &wr) noexcept
{
    return 
    [
        indexSampler = indexSamplerFrom( sources 
                                       | std::views::transform
                                       (
                                           [&](std::ranges::range_value_t<R> const &info) noexcept 
                                           {
                                               return wr(info);
                                           })
                                       ),
        &sources     = sources,
        &df = df
        ](vec3 const pos) noexcept
    { 
        auto const  [I,  muI] = indexSampler();
        auto const &[r, n, pdf] = df(sources[I].pos);
        return SourceSample
        {
            .dr = r - pos,
            .n = n,
            .pdf = pdf * muI, 
        };
    };
}
