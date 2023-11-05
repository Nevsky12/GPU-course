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

struct PyramidSample
{
    vec3 dr;
    f32 pdf;
};


f32 pyramidSolidAngle( vec3 const pos
                     , Triangle const &tr
                     ) noexcept
{
    auto const [r0, r1, r2] = tr;
/*
    vec3 const a = r0 - pos;
    vec3 const b = r1 - pos;
    vec3 const c = r2 - pos;

    auto const mix = []( vec3 const p1
                       , vec3 const p2
                       , vec3 const p3
                       ) noexcept -> f32
    {
        return dot(p1, p2) * length(p3);
    };
    */
    return 1.f / 6.f * std::abs(dot(r0 - pos, cross(r1 - r0, r2 - r0)));
    /*
    f32 const omega = 2.f * std::atan(std::abs(dot(a, cross(b, c))) 
         /
           (
               length(a) * length(b) * length(c)
               + mix(a, b, c)
               + mix(a, c, b)
               + mix(b, c, a)
           ));  
    return omega < 0.f
         ? omega + f32(M_PI)
         : omega;
         */
}


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
                                               return 0.5f * length(cross(r1 - r0, r2 - r0)) 
                                                           * length(info.emission);
                                           })
                                       ),
        &sources     = sources,
        &f = uniformTrianglePoint
        ](vec3 const p0) noexcept
    {
        auto const [I,  muI] = indexSampler();
        Triangle const &trI = sources[I].pos;
        auto const [p, pdfI] = f(trI);
        
        /*
        f32 pdfFS = muI * pdfI;
        for(unsigned i = 1u; i < sources.size(); ++i)
        {
            auto const [J,  muJ] = indexSampler();
            Triangle const &trJ = sources[J].pos;
            auto const [_, pdfJ] = f(trJ);
            pdfFS += muJ * pdfJ;
        }
        */


        
        return PyramidSample
        {
            .dr = p - p0,
            .pdf = 1.f / pyramidSolidAngle(p0, trI),
        };
    };
}
