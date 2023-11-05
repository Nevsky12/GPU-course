#include <gltf/gltf.h>
#include <gltf/as/top_level.h>
#include <gltf/as/bottom_level.h>

#include <atomic>
#include <thread>
#include <numbers>

#include <fstream>
#include <iostream>

#include <gltf/utils/aabb.h>
#include <gltf/utils/camera.h>
#include <gltf/utils/texture.h>
#include <gltf/utils/offset_point.h>
#include <gltf/utils/triangle.h>

//#define USE_EXR
#ifdef USE_EXR
#include <ImfRgbaFile.h>
#endif

using gltf::vec2;
using gltf::vec3;
using gltf::vec4;
using gltf::mat3;
using gltf::mat4x3;
using gltf::u8;
using gltf::u32;
using gltf::f32;

using namespace gltf::utils;

struct RayBLASIntersection
{
    u32 triangleI;
    RayTriangleIntersection rti;
};
struct RayTLASIntersection
{
    u32 instanceI;
    u32 geometryI;
    u32 triangleI;
    RayTriangleIntersection rti;
};

inline auto intersectBLAS(auto const &blas) noexcept
{
    return [&blas](Ray const &ray, RayDistanceRange rdr) noexcept
        -> std::optional<RayBLASIntersection>
    {
        auto const &[bvh, triangle] = blas;
        std::optional<RayBLASIntersection> result;

        auto traverse = bvh.traverse();
        while(traverse.isIncomplete())
        {
            if(traverse.onLeaf())
            {
                u32 const i = traverse.leaf();
                RayTriangleIntersection const intersection = rayTriangleIntersection(ray, triangle[i].pos, rdr);
                if(happened(intersection))
                {
                    rdr.tFar = intersection.t;
                    result = {i, intersection};
                }
                traverse = traverse.next();
            }
            else
            {
                auto const iLeft  = rayAABBIntersection(ray, bvh.box[traverse. left()], rdr);
                auto const iRight = rayAABBIntersection(ray, bvh.box[traverse.right()], rdr);
                auto const down = traverse.down(nonempty(iLeft), nonempty(iRight), iLeft.tNear <= iRight.tNear);
                traverse = down
                    ? *down
                    : traverse.next();
            }
        }
        return result;
    };
}
inline auto intersectTLAS(auto const &tlas, auto const &blas) noexcept
{
    return [&](Ray const &ray, RayDistanceRange rdr = {0.f, 1.f / 0.f}) noexcept
        -> std::optional<RayTLASIntersection>
    {
        auto const &[bvh, blasInfo, tlasInfo] = tlas;
        std::optional<RayTLASIntersection> result;

        auto traverse = bvh.traverse();
        while(traverse.isIncomplete())
        {
            if(traverse.onLeaf())
            {
                u32 const i = traverse.leaf();
                //if(tlasInfo[i].pack.opaque)
                {
                    u32 const geometryI = tlasInfo[i].pack.geometryI;
                    Ray const blasRay =
                    {
                        tlasInfo[i].transform * vec4(ray.pos, 1.f),
                        tlasInfo[i].transform * vec4(ray.dir, 0.f),
                    };
                    auto const intersection = intersectBLAS(blas[geometryI])(blasRay, rdr);
                    if(intersection)
                    {
                        rdr.tFar = intersection->rti.t;
                        result = {i, geometryI, intersection->triangleI, intersection->rti};
                    }
                }
                traverse = traverse.next();
            }
            else
            {
                auto const iLeft  = rayAABBIntersection(ray, bvh.box[traverse. left()], rdr);
                auto const iRight = rayAABBIntersection(ray, bvh.box[traverse.right()], rdr);
                auto const down = traverse.down(nonempty(iLeft), nonempty(iRight), iLeft.tNear <= iRight.tNear);
                traverse = down
                    ? *down
                    : traverse.next();
            }
        }
        return result;
    };
};

int main()
{
    gltf::GLTF const gltf(std::ifstream("../spitfire.glb", std::ios::binary));

    /* dump json:
    for(gltf::u8 const u : gltf.binary.chunk[0].data)
        std::cout << char(u);
    std::cout << std::endl;
    */

    gltf::as::TopLevel const tlas = {gltf};
    auto const blasR = tlas.blasInfo | std::views::transform
    (
        [&gltf, geometry = gltf::view::geometry(gltf)](gltf::as::TopLevel::BLASInfo const &info) noexcept
            -> gltf::as::BottomLevel<gltf::utils::AABB, gltf::view::Triangle>
        {
            gltf::Mesh::Primitive const &primitive = gltf.json.meshes[info.meshI].primitives[info.primitiveI];
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

    auto const trace = [&, intersect = intersectTLAS(tlas, blas)](Ray const ray) noexcept
    {
        const vec3 light = normalize(vec3(-1.f, 1.f, 1.f));
        const vec3 skyColor = vec3(0.5f, 0.6f, 0.9f);
        const vec3 lightColor = vec3(1.f, 0.9f, 0.6f) * 10.f;

        auto const hit = intersect(ray);
        if(!hit)
        {
            if(dot(light, ray.dir) > 0.99999f)
                return lightColor;

            if(dot(vec3(1.f, 0.f, 0.f), ray.dir) > 0.99999f)
                return vec3(1.f, 0.f, 0.f);
            if(dot(vec3(0.f, 1.f, 0.f), ray.dir) > 0.99999f)
                return vec3(0.f, 1.f, 0.f);
            if(dot(vec3(0.f, 0.f, 1.f), ray.dir) > 0.99999f)
                return vec3(0.f, 0.f, 1.f);

            return skyColor;
        }

        gltf::view::Triangle const &tri = blas[hit->geometryI].triangle[hit->triangleI];

        auto const [t, bary, frontFacing] = hit->rti;
        mat3 const R = transpose(mat3(tlas.tlasInfo[hit->instanceI].transform));
        vec3 const norm = R * interpolate(tri.norm, bary);
        vec2 const tex = interpolate(tri.tex, bary);
        vec3 const pos = ray.pos + ray.dir * t;

        bool const inShadow = static_cast<bool>(intersect({offsetPoint(pos, norm), light}));

        auto const pbr = tlas.blasInfo[hit->geometryI].materialI
                             .and_then(lambdaR1(x, gltf.json.materials[x].pbrMetallicRoughness));

        std::optional<vec4> const  texAlbedo = pbr.and_then (lambdaE1(x, x.baseColorTexture))
                                                  .transform(lambdaR1(x, textureV[x.index].sample(tex)));
        std::optional<vec4> const baseAlbedo = pbr.transform(lambdaE1(x, x.baseColorFactor));
        vec4 const albedo = texAlbedo.value_or(baseAlbedo.value_or(vec4(1.f)));

        f32 const NL = gltf::max(0.f, dot(norm, light));
        return vec3(albedo) / std::numbers::pi_v<gltf::f32> * (skyColor + lightColor * ((NL == 0.f || inShadow) ? 0.f : NL * 0.7f));
    };

    u32 const width  = 2560u;
    u32 const height = 1600u;
    auto const &[rmin, rmax] = tlas.bvh.box[0];
    Camera const camera =
    {
        {rmax.x, rmin.y, rmax.z},
        0.5f * (rmin + rmax),
        vec3{0.f, 0.f, 1.f},
        Camera::scaleFrom(0.6f, f32(width) / f32(height))
    };

#ifdef USE_EXR
    std::vector<Imf::Rgba> color(width * height);
#else
    using RGB8 = gltf::gvec<gltf::u8, 3>;
    std::vector<RGB8> color(width * height);
#endif

    std::atomic<u32> a(0);
    std::vector<std::thread> thr(std::thread::hardware_concurrency());

    for(std::thread &t : thr)
        t = std::thread([&]() noexcept
        {
            while(true)
            {
                u32 const i = a.fetch_add(1);
                if(i >= height)
                    return;

                for(u32 j = 0u; j < width ; ++j)
                {
                    auto const tonemap = +[](vec3 const c, f32 const e) noexcept
                    {
                        return liftA1(lambdaR1(x, 1.f - std::exp(-x * e)), c);
                    };

                    f32 const dx = 0.5f / f32(width  - 1u);
                    f32 const dy = 0.5f / f32(height - 1u);
                    f32 const x = -1.f + 2.f * f32(j) / f32(width  - 1u);
                    f32 const y = -1.f + 2.f * f32(i) / f32(height - 1u);

                    vec3 const c = (trace(camera.castRay({x + dx, -y + dy}))
                                 +  trace(camera.castRay({x + dx, -y - dy}))
                                 +  trace(camera.castRay({x - dx, -y + dy}))
                                 +  trace(camera.castRay({x - dx, -y - dy}))) * 0.25f;
#ifdef USE_EXR
                    auto const [r, g, b] = c;
                    color[i * width + j] = {r, g, b};
#else
                    color[i * width + j] = liftA1(encodeSRGB, tonemap(c, 1.f));
#endif
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
    std::ofstream out("out.ppm");
    out << "P6\n" << width << ' ' << height << "\n255\n";
    out.write(reinterpret_cast<char const *>(color.data()), std::streamsize(color.size() * sizeof(RGB8)));
#endif
}
