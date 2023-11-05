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
    f32 pdf;
};

f32 triangleWeight(EmissiveTriangleInfo const &info) noexcept
{
    auto const [r0, r1, r2] = info.pos;
    return 0.5f * length(cross(r2 - r0, r1 - r0)) * length(info.emission);
}

template<std::ranges::range R, typename DF, typename Weighter>
requires( requires( std::ranges::range_value_t<R> const r
                  , Weighter const wr
                  , DF const df
                  ) 
              {
                  {r.pos} -> std::convertible_to<vec3>;
                  {wr(r)} -> std::convertible_to<f32>;
                  {df(r.pos, vec3{}, vec3{})};
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
        ](vec3 const pos, vec3 const norm) noexcept
    {
        /*     
        f32 pdfFS = 0.f;
        for(unsigned i = 0u; i < sources.size(); ++i)
        {
            u32 k = u32(sources.size());
            f32 muJ = 0.f;
            do
            {
               auto const [m, muM] = indexSampler();
               k = m;
               muJ = muM;
            }
            while(k != i);
            Triangle const &trJ = sources[k].pos;
            pdfFS += muJ * uniformTrianglePoint(trJ, pos, norm).pdf;
        }
        */
                
        auto const  [I,  muI] = indexSampler();
        auto const &[r, pdf] = df(sources[I].pos, pos, norm);
        return SourceSample
        {
            .dr = r - pos,
            .pdf = pdf * muI, 
        };
    };
}
