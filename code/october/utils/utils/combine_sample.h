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


struct TriangleSource
{
    vec3 r;
    f32 pdf;
};


template<typename IS>
auto triangleSourceSample( IS &&indexSampler
                         , std::vector<EmissiveTriangleInfo> const &sources
                         ) noexcept
{
    return 
    [
         indexSampler  =  std::move(indexSampler),
        &sources       =  sources
    ](vec3 const rayOrigin, vec3 const rayNorm) noexcept
    -> TriangleSource
    {
        auto const  [I, probI] = indexSampler();
        auto const &[p,   pdf] = uniformTrianglePoint(sources[I].pos, rayOrigin, rayNorm); 
        return 
        {
            .r      = p,
            .pdf    = probI * pdf,
        };
    };
}


/*
struct SourceSample
{
    vec3 dr;
    f32 pdf;
};


f32 triangleWeight(EmissiveTriangleInfo const &info) noexcept
{
    auto const [r0, r1, r2] = info.pos;
    auto const [r, g, b] = info.emission;
    f32 const luminance = 0.2126f * r + 0.7152f * g + 0.0722f * b;
    return length(cross(r2 - r0, r1 - r0)) * luminance;
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
    auto const indexSampler =  
                       indexSamplerFrom( sources 
                                       | std::views::transform
                                       (
                                           [&](std::ranges::range_value_t<R> const &info) noexcept 
                                           {
                                               return wr(info);
                                           })
                                       );
    return 
    [
        indexSampler = indexSampler,
        &sources     = sources,
        &df = df,
        pdfFS = [&]() noexcept -> f32
        {
            f32 pdfFS = 0.f;
            for(u32 i = 0u; i < sources.size(); ++i)
            {
                auto const [J, muJ] = indexSampler();
                auto const [un1, un2, pdfJ] = df(sources[J].pos);
                pdfFS += muJ * pdfJ;
            }
            return pdfFS;
        }()
        ](vec3 const pos) noexcept
    { 
        auto const  [I,    muI] = indexSampler();
        auto const &[r, n, pdf] = df(sources[I].pos);
        return SourceSample
        {
            .dr = r - pos,
            .n = n,
            .pdf = pdfFS / pdf / muI, 
        };
    };
}
*/
