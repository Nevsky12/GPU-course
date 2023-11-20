#pragma once
#include "tlas.h"

struct Scene
{
    gltf::GLTF gltf;

    gltf::as::TopLevel tlas;

    using BLAS = gltf::as::BottomLevel<gltf::utils::AABB, gltf::view::Triangle>;
    std::vector<BLAS> blas;

    Scene(gltf::GLTF &&scene) noexcept
        : gltf(static_cast<gltf::GLTF &&>(scene))
        , tlas(gltf)
    {
        auto const blasR = tlas.blasInfo | std::views::transform
        (
            [this](gltf::as::TopLevel::BLASInfo const &info) noexcept
                -> gltf::as::BottomLevel<gltf::utils::AABB, gltf::view::Triangle>
            {
                gltf::Mesh::Primitive const &primitive = gltf.json.meshes    [info.meshI]
                                                                  .primitives[info.primitiveI];
                return {gltf::view::geometry(gltf)(primitive).second, lambdaE1(x, x), lambdaE1(x, x)};
            }
        );
        blas = std::vector<BLAS>
        {
            std::ranges::begin(blasR),
            std::ranges::  end(blasR),
        };
    }

    struct SurfacePoint
    {
        vec3 pos;
        vec2 tex;
        vec3 norm;
        RayTLASIntersection intersection;
    };

    struct SurfacePointSample
    {
        SurfacePoint point;
        f32 pdfW; // projected solid angle PDF
    };

    std::optional<SurfacePoint> closestHit(Ray const ray, RayDistanceRange const rdr = {0.f, 1.f / 0.f}) const noexcept
    {
        return intersectTLAS(tlas, blas)(ray, rdr).transform
        (
            [this, ray](RayTLASIntersection const hit) noexcept
            {
                gltf::view::Triangle const &tri = blas[hit.geometryI].triangle[hit.triangleI];
                auto const [t, bary, frontFacing] = hit.rti;
                mat3 const R = transpose(mat3(tlas.tlasInfo[hit.instanceI].transform));

                return SurfacePoint
                {
                    .pos = ray.pos + ray.dir * t,
                    .tex = interpolate(tri.tex, bary),
                    .norm = R * normalize(interpolate(tri.norm, bary)),
                    .intersection = hit,
                };
            }
        );
    }
};
