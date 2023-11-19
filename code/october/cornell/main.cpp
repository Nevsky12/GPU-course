#include <utils/tlas.h>
#include <utils/triangle_sample.h>
#include <utils/index_sample.h>
#include <utils/sample.h>

#include <gltf/utils/aabb.h>
#include <gltf/utils/camera.h>
#include <gltf/utils/texture.h>
#include <gltf/utils/offset_point.h>
#include <gltf/utils/orthonormal.h>
#include <gltf/utils/triangle.h>

#include <ImfRgbaFile.h>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <thread>
int main()
{
    gltf::GLTF const gltf(std::ifstream("../cornell.glb", std::ios::binary));

    /* dump json:
    for(gltf::u8 const u : gltf.binary.chunk[0].data)
        std::cout << char(u);
    std::cout << std::endl;
    */

    gltf::as::TopLevel const tlas = {gltf};
    auto const blasR = tlas.blasInfo | std::views::transform
    (
        [&gltf](gltf::as::TopLevel::BLASInfo const &info) noexcept
            -> gltf::as::BottomLevel<gltf::utils::AABB, gltf::view::Triangle>
        {
            gltf::Mesh::Primitive const &primitive = gltf.json.meshes    [info.meshI]
                                                              .primitives[info.primitiveI];
            return {gltf::view::geometry(gltf)(primitive).second, lambdaE1(x, x), lambdaE1(x, x)};
        }
    );
    std::vector<gltf::as::BottomLevel<gltf::utils::AABB, gltf::view::Triangle>> const blas =
    {
        std::ranges::begin(blasR),
        std::ranges::  end(blasR),
    };

    auto emissiveTLASR = tlas.tlasInfo | std::views::filter
    (
        [&gltf](gltf::as::TopLevel::Info const &info) noexcept
        {
            vec3 const e = gltf.json.materials[info.pack.materialI].emissiveFactor;
            return dot(e, e) > 0.f;
        }
    );
    struct EmissiveTriangleInfo
    {
        gltf::gvec<vec3, 3> pos;
        vec3 emission;
        u32 instanceI;
        u32 triangleI;
    };
    auto emissiveTriangleR = emissiveTLASR | std::views::enumerate
                                           | std::views::transform
    (
        [&](auto const &pair) noexcept
        {
            auto const &[instanceI, info] = pair;

            vec3 const emission = gltf.json.materials[info.pack.materialI].extensions
                 .and_then(lambdaE1(x, x.KHR_materials_emissive_strength))
                 .transform(lambdaE1(x, x.emissiveStrength))
                 .value_or(1.f) * gltf.json.materials[info.pack.materialI].emissiveFactor;

            auto const [m0, m1, m2, m3] = info.transform;
            mat4x3 const m = mat4x3(inverse(mat4(vec4(m0, 0.f), vec4(m1, 0.f), vec4(m2, 0.f), vec4(m3, 1.f))));
            return blas[info.pack.geometryI].triangle | std::views::enumerate
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
    std::vector<EmissiveTriangleInfo> const emissiveTriangleInfo =
    {
        std::ranges::begin(emissiveTriangleR),
        std::ranges::  end(emissiveTriangleR),
    };
    auto const emissivePointSample =
    [
        &emissiveTriangleInfo,
        indexSampler = indexSamplerFrom(emissiveTriangleInfo | std::views::transform
            (
                [](EmissiveTriangleInfo const &info) noexcept
                {
                    auto const [p0, p1, p2] = info.pos;
                    auto const [r, g, b] = info.emission;
                    f32 const luminance = 0.2126f * r + 0.7152f * g + 0.0722f * b;
                    return length(cross(p1 - p0, p2 - p0)) * luminance;
                }
            )
        )
    ](vec3 const pos, vec3 const norm) noexcept
    {
        auto const [i, pdfi] = indexSampler();
        auto const [p, pdft] = uniformTrianglePoint(emissiveTriangleInfo[i].pos, pos, norm);
        return std::tuple{p, emissiveTriangleInfo[i], pdfi * pdft};
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
        RayTLASIntersection intersection;
    };
    auto const closestHit = [&, intersect = intersectTLAS(tlas, blas)](Ray const ray, RayDistanceRange const rdr = {1e-3f, 1.f / 0.f}) noexcept
    {
        return intersect(ray, rdr).transform
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
                vec3 const emission = material.transform(lambdaE1(x, x.emissiveFactor))
                                              .value_or(vec3(0.f));
                f32 const emissionStrength = material.and_then(lambdaE1(x, x.extensions))
                                                     .and_then(lambdaE1(x, x.KHR_materials_emissive_strength))
                                                     .transform(lambdaE1(x, x.emissiveStrength))
                                                     .value_or(1.f);

                auto const pbr = material.and_then(lambdaE1(x, x.pbrMetallicRoughness));
                std::optional<vec4> const  texAlbedo = pbr.and_then (lambdaE1(x, x.baseColorTexture))
                                                          .transform(lambdaR1(x, textureV[x.index].sample(tex)));
                std::optional<vec4> const baseAlbedo = pbr.transform(lambdaE1(x, x.baseColorFactor));

                return Hit
                {
                    .pos = ray.pos + ray.dir * t,
                    .norm = R * interpolate(tri.norm, bary),
                    .albedo = {texAlbedo.value_or(baseAlbedo.value_or(vec4(1.f)))},
                    .emission = emission * emissionStrength,
                    .intersection = hit,
                };
            }
        );
    };


    auto const trace = [&](Ray const cameraRay) noexcept
    {
        vec3 const skyL = vec3(0.f); // steady sky
                              
        auto const weightsFrom = [](f32 const w1, f32 const w2) noexcept
        {
            f32 const p21 = w1 * w1;
            f32 const p22 = w2 * w2;
            return vec2{p21 / (p21 + p22), p22 / (p21 + p22)};
        };

        auto const impl = [&](Ray const ray, f32 const HP, auto const &self) noexcept
        {
            auto const hit = closestHit(ray);
            if(!hit)
                return skyL;

            auto const [pos, norm, albedo, Le, intersection] = *hit;

            auto const [point, tri, pdfLightSource] = emissivePointSample(pos, norm);
            vec3 const wiLS = normalize(point - pos);

            auto const secondaryHit = closestHit({pos + 1e-4f * wiLS, wiLS});
            vec3 const real =  secondaryHit ? secondaryHit->pos : skyL;
            bool const same = (secondaryHit && dot(real - point, real - point) < 1e-8f);


            auto const [z, pdfBSDF] = uniformProjectedHemisphereSample();
            vec3 const wiBSDF = orthonormal(norm) * z;

            vec2 const ws = weightsFrom(pdfLightSource, pdfBSDF);

            f32 const limiters[] = {1.f - HP, HP};
            auto const [indicator, prob] = indexSamplerFrom(limiters)();

            auto const Li = [&](Ray const r) noexcept {return self(r, HP * (albedo.x + albedo.y + albedo.z) / 3.f, self);};
            return Le + (albedo / std::numbers::pi_v<f32>) 
                 * ( indicator
                     ? ( (same ? ws[0] * Li({offsetPoint(pos, norm), wiLS  }) / pdfLightSource : skyL)
                       +         ws[1] * Li({offsetPoint(pos, norm), wiBSDF}) / pdfBSDF
                     ) : skyL
                   ) / prob;
        };
        return impl(cameraRay, 1.f, impl);
    };

    u32 const width  = 1600u;
    u32 const height = 1600u;

    Camera const camera =
    {
        vec3{-3.9f, 0.f, 0.f},
        vec3{0.f, 0.f, 0.f},
        vec3{0.f, 0.f, 1.f},
        Camera::scaleFrom(0.3456f, f32(width) / f32(height))
    };

    std::vector<Imf::Rgba> color(width * height);
    std::atomic<u32> a(0);
    std::vector<std::thread> thr(std::thread::hardware_concurrency());

    for(std::thread &t : thr)
        t = std::thread([&]() noexcept
        {
            while(true)
            {
                u32 const y = a.fetch_add(1);
                if(y >= height)
                    return;

                for(u32 x = 0u; x < width; ++x)
                {
                    auto const sample = [=](u32) noexcept
                    {
                        f32 const u = -1.f + 2.f * (generateUniformFloat() + f32(x)) / f32( width);
                        f32 const v =  1.f - 2.f * (generateUniformFloat() + f32(y)) / f32(height);
                        return trace(camera.castRay({u, v}));
                    };
                    u32 const N = 10u;
                    auto const samples = std::views::iota(0u, N) | std::views::transform(sample);
                    vec3 const c = std::accumulate(std::ranges::begin(samples), std::ranges::end(samples), vec3(0.f)) / f32(N);
                    color[x + y * width] = {c.x, c.y, c.z, 1.f};
                }
            }
        });
    for(std::thread &t : thr)
        t.join();

    Imf::RgbaOutputFile file("out.exr", width, height, Imf::WRITE_RGBA);
    file.setFrameBuffer(color.data(), 1, width);
    file.writePixels(height);
}
