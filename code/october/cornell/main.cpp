#include <utils/bsdf_sampler.h>
#include <utils/light_sampler.h>

#include <gltf/utils/camera.h>
#include <gltf/utils/orthonormal.h>

#define USE_EXR
#ifdef USE_EXR
#include <ImfRgbaFile.h>
#else
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#endif

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <thread>
#include <mutex>
int main()
{
    Scene const scene(gltf::GLTF(std::ifstream("../cornell1.glb", std::ios::binary)));

    LightSampler const lightSampler(scene);
     BSDFSampler const  bsdfSampler(scene);

    auto const trace = [&](Ray const cameraRay) noexcept
    {
        vec3 const skyL = vec3(0.f);

        auto const luminance = [&](Scene::SurfacePoint const &q1, Scene::SurfacePoint const &q2, auto const &self) noexcept
            -> vec3
        {
            auto const L = [&](Scene::SurfacePoint const &a, Scene::SurfacePoint const &b) noexcept -> vec3 {return self(a, b, self);};

            auto const lightSample = lightSampler.sample(q1, q2).transform
            (
                [&](Scene::SurfacePointSample const &sample) noexcept
                {
                    auto const &[q0, pdfLS] = sample;
                    f32 const pdfBSDF = bsdfSampler.pdfW(q0, q1, q2);
                    return pdfLS / (pdfLS + pdfBSDF) * bsdfSampler.BSDF(q0, q1, q2) * lightSampler.emission(q0, q1) / pdfLS;
                }
            ).value_or(vec3(0.f));

            auto const bsdfSample = bsdfSampler.sample(q1, q2);
            return bsdfSample.transform
            (
                [&](Scene::SurfacePointSample const &sample) noexcept
                {
                    auto const &[q0, pdfBSDF] = sample;
                    f32 const pdfLS = lightSampler.pdfW(q0, q1, q2);
                    return lightSample + pdfBSDF / (pdfBSDF + pdfLS) * bsdfSampler.BSDF(q0, q1, q2) * (lightSampler.emission(q0, q1) + L(q0, q1)) / pdfBSDF;
                }
            ).value_or(skyL);
        };

        std::optional<Scene::SurfacePoint> const hit = scene.closestHit(cameraRay);
        return hit.transform
        (
            [&](Scene::SurfacePoint const &q1) noexcept
            {
                Scene::SurfacePoint const q2 = {cameraRay.pos, {}, cameraRay.dir, noTLASIntersection};
                return lightSampler.emission(q1, q2) + luminance(q1, q2, luminance);
            }
        ).value_or(skyL);
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

#ifdef USE_EXR
    using Color = Imf::Rgba;
#else
    using Color = vec4;
#endif

    std::vector<Color> color(width * height);
    std::atomic<u32> a(0);
    std::vector<std::thread> thr(std::thread::hardware_concurrency());
    std::mutex writeMutex;

    for(std::thread &t : thr)
        t = std::thread([&]() noexcept
        {
            while(true)
            {
                u32 const y = a.fetch_add(1);
                if(y >= height)
                    return;
                {
                    std::unique_lock<std::mutex> const lock(writeMutex);
                    std::cerr << std::setw(5) << y << " / " << height << '\r';
                }

                for(u32 x = 0u; x < width; ++x)
                {
                    auto const sample = [=](u32) noexcept
                    {
                        f32 const u = -1.f + 2.f * (generateUniformFloat() + f32(x)) / f32( width);
                        f32 const v =  1.f - 2.f * (generateUniformFloat() + f32(y)) / f32(height);
                        return trace(camera.castRay({u, v}));
                    };
                    u32 const N = 64u;
                    auto const samples = std::views::iota(0u, N) | std::views::transform(sample);
                    vec3 const c = std::accumulate(std::ranges::begin(samples), std::ranges::end(samples), vec3(0.f)) / f32(N);
                    color[x + y * width] = {c.x, c.y, c.z, 1.f};
                }
            }
        });
    for(std::thread &t : thr)
        t.join();


#ifdef USE_EXR
    Imf::RgbaOutputFile file("out.exr", width, height, Imf::WRITE_RGBA);
    file.setFrameBuffer(color.data(), 1, width);
    file.writePixels(height);
#else
    stbi_write_hdr("out.hdr", width, height, 4, reinterpret_cast<f32 const *>(color.data()));
#endif
}
