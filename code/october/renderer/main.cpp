#include <utils/tlas.h>
#include <utils/sample.h>

#include <gltf/utils/aabb.h>
#include <gltf/utils/camera.h>
#include <gltf/utils/texture.h>
#include <gltf/utils/offset_point.h>
#include <gltf/utils/orthonormal.h>
#include <gltf/utils/triangle.h>

#include <algorithm>
#include <iostream>
#include <fstream>
int main()
{
    gltf::GLTF const gltf(std::ifstream("../spitfire.glb", std::ios::binary));

    gltf::as::TopLevel const tlas = {gltf};
    auto const blasR = tlas.blasInfo | std::views::transform
    (
        [&gltf, geometry = gltf::view::geometry(gltf)](gltf::as::TopLevel::BLASInfo const &info) noexcept
            -> gltf::as::BottomLevel<gltf::utils::AABB, gltf::view::Triangle>
        {
            gltf::Mesh::Primitive const &primitive = gltf.json.meshes    [info.meshI]
                                                              .primitives[info.primitiveI];
            return {geometry(primitive).second, lambdaE1(x, x), lambdaE1(x, x)};
        }
    );
    std::vector<gltf::as::BottomLevel<gltf::utils::AABB, gltf::view::Triangle>> const blas =
    {
        std::ranges::begin(blasR),
        std::ranges::  end(blasR),
    };

    using Comp = f32;
    auto const imageR = gltf.json.images | std::views::transform
    (
        [&](gltf::Image const &image) noexcept
            -> gltf::utils::Image<Comp>
        {
            return {gltf, image};
        }
    );
    std::vector<gltf::utils::Image<Comp>> const image = {std::ranges::begin(imageR), std::ranges::end(imageR)};
    auto const textureR = std::views::iota(0u, u32(gltf.json.textures.size())) | std::views::transform
    (
        [&](u32 const i) noexcept
            -> gltf::utils::Texture<Comp>
        {
            gltf::Texture const &texture = gltf.json.textures[i];
            return {gltf.json.samplers[texture.sampler.value_or(0u)], image[texture.source]};
        }
    );
    std::vector<gltf::utils::Texture<Comp>> const textureV = {std::ranges::begin(textureR), std::ranges::end(textureR)};

    struct Hit
    {
        vec3 pos;
        vec3 norm;
        vec3 albedo;
        vec3 emission;
    };
    auto const closestHit = [&, intersect = intersectTLAS(tlas, blas)](Ray const ray) noexcept
    {
        return intersect(ray).transform
        (
            [&](RayTLASIntersection const hit) noexcept
            {
                gltf::view::Triangle const &tri = blas[hit.geometryI].triangle[hit.triangleI];

                auto const [t, bary, frontFacing] = hit.rti;
                mat3 const R = transpose(mat3(tlas.tlasInfo[hit.instanceI].transform));
                vec2 const tex = interpolate(tri.tex, bary);

                auto const material = tlas.blasInfo[hit.geometryI]
                                          .materialI
                                          .transform([&](u32 const i) {return gltf.json.materials[i];});
                std::optional<vec3> const emission = material.transform(lambdaE1(x, x.emissiveFactor));

                auto const pbr = material.and_then(lambdaE1(x, x.pbrMetallicRoughness));
                std::optional<vec4> const  texAlbedo = pbr.and_then (lambdaE1(x, x.baseColorTexture))
                                                          .transform(lambdaR1(x, textureV[x.index].sample(tex)));
                std::optional<vec4> const baseAlbedo = pbr.transform(lambdaE1(x, x.baseColorFactor));

                return Hit
                {
                    .pos = ray.pos + ray.dir * t,
                    .norm = R * interpolate(tri.norm, bary),
                    .albedo = {texAlbedo.value_or(baseAlbedo.value_or(vec4(1.f)))},
                    .emission = emission.value_or(vec3(0.f)),
                };
            }
        );
    };
    auto const trace = [&](Ray const ray) noexcept
    {
        vec3 const skyL = vec3(0.5f, 0.8f, 1.f) * 0.02f;

        auto const hit = closestHit(ray);
        if(!hit)
            return skyL;

        auto const sample = [&](u32) noexcept
        {
            auto const [pos, norm, albedo, emission] = *hit;
            auto const [z, pdf] = uniformProjectedHemisphereSample();

            vec3 const wi = orthonormal(norm) * z;
            auto const secondaryHit = closestHit({offsetPoint(pos, norm), wi});

            vec3 const L = secondaryHit ? secondaryHit->emission : skyL;
            return L * (albedo / std::numbers::pi_v<f32>) / pdf;
        };

        u32 const N = 64u;
        auto const sum = *std::ranges::fold_left_first(std::views::iota(0u, N) | std::views::transform(sample), lambdaE2(x, y, x + y));
        return hit->emission + sum / f32(N);
    };

    std::ofstream file("out.ppm");
    u32 const width  = 1920u;
    u32 const height = 1080u;
    file << "P3\n" << width << ' ' << height << "\n255\n";

    auto const &[rmin, rmax] = tlas.bvh.box[0];
    Camera const camera =
    {
        {rmax.x, rmin.y, rmax.z},
        0.5f * (rmin + rmax),
        vec3{0.f, 0.f, 1.f},
        Camera::scaleFrom(0.6f, f32(width) / f32(height))
    };
    /*
    Camera const camera =
    {
        {0.f, -3.9f, 0.f},
        {0.f, 0.f, 0.f},
        vec3{0.f, 0.f, 1.f},
        Camera::scaleFrom(0.35f, f32(width) / f32(height))
    };
    */

    for(u32 y = 0u; y < height; ++y)
    for(u32 x = 0u; x <  width; ++x)
    {
        f32 const u = -1.f + 2.f * (0.5f + f32(x)) / f32( width);
        f32 const v =  1.f - 2.f * (0.5f + f32(y)) / f32(height);
        auto const [r, g, b] = trace(camera.castRay({u, v}));

        auto const tonemap = +[](f32 const f) noexcept
        {
            float const exposure = 5.f;
            return 1.f - std::exp(-f * exposure);
        };
        file << int(encodeSRGB(tonemap(r))) << ' '
             << int(encodeSRGB(tonemap(g))) << ' '
             << int(encodeSRGB(tonemap(b))) << ' ';
    }
}
