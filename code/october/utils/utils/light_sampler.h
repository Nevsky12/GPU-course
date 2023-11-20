#pragma once
#include "sample.h"
#include "scene.h"

class LightSampler
{
    Scene const &scene;
    struct EmissiveTriangleInfo
    {
        gltf::gvec<vec3, 3> pos;
        vec3 emission;
        u32 instanceI;
        u32 triangleI;
    };
    std::vector<EmissiveTriangleInfo> emissiveTriangleInfo;
    std::vector<f32> triangleCDF;

    f32 trianglePDF(u32 const i) const noexcept {return triangleCDF[i + 1] - triangleCDF[i];}
    u64 triangleID(u32 const instanceI, u32 const triangleI) const noexcept
    {
        return (u64(instanceI) << 32u) | u64(triangleI);
    }
    u64 triangleID(Scene::SurfacePoint const &point) const noexcept
    {
        return triangleID(point.intersection.instanceI, point.intersection.triangleI);
    }
    std::optional<u32> findTriangleIndex(Scene::SurfacePoint const &point) const noexcept
    {
        u64 const I = triangleID(point);
        auto const proj = lambdaR1(info, triangleID(info.instanceI, info.triangleI));
        auto const it = std::ranges::lower_bound(emissiveTriangleInfo, I, {}, proj);
        if(it == emissiveTriangleInfo.end() || proj(*it) != I)
            return std::nullopt;
        return {u32(it - emissiveTriangleInfo.begin())};
    }

public:
    LightSampler(Scene const &sceneCRef) noexcept
        : scene(sceneCRef)
    {
        auto emissiveTLASR = scene.tlas.tlasInfo | std::views::enumerate
                                                 | std::views::filter
        (
            [this](auto const &pair) noexcept
            {
                auto const &[instanceI, info] = pair;
                vec3 const e = scene.gltf.json.materials[info.pack.materialI].emissiveFactor;
                return dot(e, e) > 0.f;
            }
        );
        auto emissiveTriangleR = emissiveTLASR | std::views::transform
        (
            [this](auto const &pair) noexcept
            {
                auto const &[instanceI, info] = pair;

                vec3 const emission = scene.gltf.json.materials[info.pack.materialI].extensions
                     .and_then (lambdaE1(x, x.KHR_materials_emissive_strength))
                     .transform(lambdaE1(x, x.emissiveStrength))
                     .value_or(1.f) * scene.gltf.json.materials[info.pack.materialI].emissiveFactor;

                auto const [m0, m1, m2, m3] = info.transform;
                mat4x3 const m = mat4x3(inverse(mat4(vec4(m0, 0.f), vec4(m1, 0.f), vec4(m2, 0.f), vec4(m3, 1.f))));
                return scene.blas[info.pack.geometryI].triangle | std::views::enumerate
                                                                | std::views::transform
                (
                    [emission, instanceI, m](auto const &anotherPair) noexcept
                    {
                        auto const &[triangleI, triangle] = anotherPair;
                        return EmissiveTriangleInfo
                        {
                            .pos = liftA1(lambdaR1(r, m * vec4(r, 1.f)), triangle.pos),
                            .emission = emission,
                            .instanceI = u32(instanceI),
                            .triangleI = u32(triangleI),
                        };
                    }
                );
            }
        ) | std::views::join | std::views::common;

        emissiveTriangleInfo = std::vector<EmissiveTriangleInfo>
        {
            std::ranges::begin(emissiveTriangleR),
            std::ranges::  end(emissiveTriangleR),
        };
        auto const triangleWeightR = emissiveTriangleInfo | std::views::transform
        (
            [](EmissiveTriangleInfo const &info) noexcept
            {
                auto const [p0, p1, p2] = info.pos;
                f32 const luminance = dot(vec3(0.2126f, 0.7152f, 0.0722f), info.emission);
                return length(cross(p1 - p0, p2 - p0)) * luminance;
            }
        );
        f64 const c = std::accumulate(std::ranges::begin(triangleWeightR), std::ranges::end(triangleWeightR), 0.);

        triangleCDF = std::vector<f32>{0.f};
        f64 sum = 0.;
        for(f64 const w : triangleWeightR)
        {
            sum += w / c;
            triangleCDF.push_back(f32(sum));
        }
    }

    // light flow direction: q0 -> q1 -> q2
    vec3 emission(Scene::SurfacePoint const &q1, Scene::SurfacePoint const &/*q2*/) const noexcept
    {
        return findTriangleIndex(q1).transform(lambdaR1(i, emissiveTriangleInfo[i].emission))
                                    .value_or(vec3(0.f));
    }
    std::optional<Scene::SurfacePointSample> sample(Scene::SurfacePoint const &q1, Scene::SurfacePoint const &/*q2*/) const noexcept
    {
        auto const it = std::ranges::upper_bound(triangleCDF, generateUniformFloat());
        u32 const i = u32(it - triangleCDF.begin()) - 1u;
        f32 const pdfi = trianglePDF(i);

        auto const &[tri, emission, instanceI, triangleI] = emissiveTriangleInfo[i];

        auto const &[r0, r1, r2] = tri;
        vec3 const p = uniformTrianglePoint(tri);
        vec3 const dr = p - q1.pos;
        vec3 const wi = gltf::normalize(dr);

        f32 const pdft = dot(dr, dr) * dot(dr, dr)
             / gltf::abs(dot(q1.norm, dr) * dot(dr, cross(r1 - r0, r2 - r0)) * 0.5f);

        return scene.closestHit({q1.pos, wi}, {1e-5f / dot(q1.norm, wi), dot(dr, wi) * 1.0001f}).and_then
        (
            [&](Scene::SurfacePoint const &q0) noexcept
                -> std::optional<Scene::SurfacePointSample>
            {
                if(triangleID(q0) != triangleID(instanceI, triangleI))
                    return std::nullopt;
                return Scene::SurfacePointSample{q0, pdfi * pdft};
            }
        );
    }
    f32 pdfW(Scene::SurfacePoint const &q0, Scene::SurfacePoint const &q1, Scene::SurfacePoint const &/*q2*/) const noexcept
    {
        return findTriangleIndex(q0).transform
        (
            [&, this](u32 const i) noexcept
            {
                vec3 const dr = q0.pos - q1.pos;
                vec3 const wi = normalize(dr);
                std::optional<Scene::SurfacePoint> const q0Opt = scene.closestHit({q1.pos, wi}, {1e-5f / dot(q1.norm, wi), dot(dr, wi) * 1.0001f});
                if(!q0Opt || triangleID(*q0Opt) != triangleID(q0))
                    return 0.f;

                auto const &[tri, emission, instanceI, triangleI] = emissiveTriangleInfo[i];
                auto const &[r0, r1, r2] = tri;

                f32 const pdfi = trianglePDF(i);
                f32 const pdft = dot(dr, dr) * dot(dr, dr)
                               / gltf::abs(dot(q1.norm, dr) * dot(dr, cross(r1 - r0, r2 - r0)) * 0.5f);
                return pdfi * pdft;
            }
        ).value_or(0.f);
    }
};
