#pragma once
#include "sample.h"
#include "scene.h"
#include <gltf/utils/texture.h>

struct BSDFSampler
{
    Scene const &scene;
    std::vector<std::optional<gltf::Material::PBRMetallicRoughness>> material; // geometryI -> Material
    std::vector<gltf::utils::Image  <f32>> image;
    std::vector<gltf::utils::Texture<f32>> texture;

    BSDFSampler(Scene const &sceneCRef) noexcept
        : scene(sceneCRef)
    {
        auto const materialR = scene.tlas.blasInfo | std::views::transform
        (
            [this](auto const &info) noexcept
            {
                return info.materialI.and_then(lambdaR1(i, scene.gltf.json.materials[i].pbrMetallicRoughness));
            }
        );
        material = std::vector<std::optional<gltf::Material::PBRMetallicRoughness>>
        {
            std::ranges::begin(materialR),
            std::ranges::  end(materialR),
        };

        auto const imageR = scene.gltf.json.images | std::views::transform
        (
            [this](gltf::Image const &img) noexcept
                -> gltf::utils::Image<f32>
            {
                return {scene.gltf, img};
            }
        );
        image = std::vector<gltf::utils::Image<f32>>{std::ranges::begin(imageR), std::ranges::end(imageR)};

        auto const textureR = std::views::iota(0u, u32(scene.gltf.json.textures.size())) | std::views::transform
        (
            [this](u32 const i) noexcept
                -> gltf::utils::Texture<f32>
            {
                gltf::Texture const &tex = scene.gltf.json.textures[i];
                return {scene.gltf.json.samplers[tex.sampler.value_or(0u)], image[tex.source]};
            }
        );
        texture = std::vector<gltf::utils::Texture<f32>>{std::ranges::begin(textureR), std::ranges::end(textureR)};
    }

    // light flow direction: q0 -> q1 -> q2
    vec3 BSDF(Scene::SurfacePoint const &/*q0*/, Scene::SurfacePoint const &q1, Scene::SurfacePoint const &/*q2*/) const noexcept
    {
        auto const [instanceI, geometryI, triangleI, rti] = q1.intersection;
        auto const pbr = *material[geometryI];
        vec4 const baseColor = pbr.baseColorTexture
                                  .transform(lambdaR1(x, texture[x.index].sample(q1.tex)))
                                  .value_or(vec4(1.f));
        vec3 const albedo = baseColor * pbr.baseColorFactor;

        return albedo * std::numbers::inv_pi_v<f32>;
    }
    std::optional<Scene::SurfacePointSample> sample(Scene::SurfacePoint const &q1, Scene::SurfacePoint const &q2) const noexcept
    {
        vec3 const wi = uniformProjectedHemisphereSample(q1.norm);
        return scene.closestHit({q1.pos, wi}, {1e-5f / dot(q1.norm, wi), 1.f / 0.f}).transform
        (
            [&](Scene::SurfacePoint const &q0) noexcept
                -> Scene::SurfacePointSample
            {
                return {q0, pdfW(q0, q1, q2)};
            }
        );
    }
    f32 pdfW(Scene::SurfacePoint const &/*q0*/, Scene::SurfacePoint const &/*q1*/, Scene::SurfacePoint const &/*q2*/) const noexcept
    {
        return std::numbers::inv_pi_v<f32>;
    }
};
