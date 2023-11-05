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

auto sourcesSamplerFrom(std::vector<EmissiveTriangleInfo> const &sources) noexcept
{
    return 
    [
        indexSampler = indexSamplerFrom( sources 
                                       | std::views::transform
                                       (
                                           [](EmissiveTriangleInfo const &info) noexcept 
                                           {
                                               auto const [r0, r1, r2] = info.pos;
                                               return 0.5f * length(cross(r2 - r0, r1 - r0)) * length(info.emission);
                                           })
                                       ),
        &sources     = sources,
        &f = uniformTrianglePoint
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
        auto const &[r, pdf] = uniformTrianglePoint(sources[I].pos, pos, norm);
        return SourceSample
        {
            .dr = r - pos,
            .pdf = pdf * muI, 
        };
    };
}
