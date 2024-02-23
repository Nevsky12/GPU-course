#include <algorithm>
#include <fstream>
#include <ranges>

#include <utils/camera.h>
#include <utils/sphere.h>
#include <utils/overloaded.h>

#include <iostream>

#   define ASSERT(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
                      << " line " << __LINE__ << ": " << message << std::endl; \
            std::terminate(); \
        } \
    } while (false)



int main()
{
    using vvec3 = vec3x4;
    using vf32  =  f32x4;
    u32 const chunkSize = vf32::size();

    std::vector<Sphere<vec3>> sphere;
    u32 const nx = 15;
    u32 const ny = 10;
    f32 const step = 0.1f;

    for(u32 i = 0u; i < nx; ++i)
    for(u32 j = 0u; j < ny; ++j)
        sphere.push_back
        (
            {
                .origin = {2.f + -f32(i) * step * 3, 1.f + -f32(j) * step * 3, 0.f},
                .radius = step,
            }
        );
   
    std::vector<vec3> albedo;
    for(u32 i = 0u; i < nx; ++i)
    for(u32 j = 0u; j < ny; ++j)
        albedo.push_back
        (
            {i * step, (i * nx + j) * step, 0.f}
        );

    assert(sphere.size() <= albedo.size());

    auto const vectorizePrimitive = [chunkSize](std::vector<Sphere<vec3>> const &raw) noexcept 
    {
        std::vector<Sphere<vec3x4>> res;

        Sphere<vec3x4> tmpSph;

        for(auto const chunk: raw | std::views::chunk(chunkSize))
        {
            for(auto const &[j, sph]: chunk | std::views::enumerate)
            {
                for(u32 i = 0u; i < 3u; ++i)
                    tmpSph.origin[i][j] = sph.origin[i];
                tmpSph.radius[j] = sph.radius;
            }
            res.push_back(tmpSph);
            tmpSph = 
            {
                .origin = {0.f},
                .radius =  0.f,
            };
        }

        return res;
    };

    struct Hit
    {
        u32 sphereI;
        f32 t;
    };
    Hit const nullHit = {-1u, -1.f / 0.f};
   
    auto const miss = [](Hit const &h) noexcept {return (h.sphereI == -1u);};

    std::vector<Sphere<vec3x4>> const vectorizedSpheres = vectorizePrimitive(sphere);

    /*
    for(auto const [origin, radius]: vectorizedSpheres)
    {
        std::cout << "origin: ";
        for(int i = 0; i < 4; ++i)
            std::cout << origin.x[i] << ", ";
        std::cout << std::endl;
        for(int i = 0; i < 4; ++i)
            std::cout << origin.y[i] << ", ";
        std::cout << std::endl;
        for(int i = 0; i < 4; ++i)
            std::cout << origin.z[i] << ", ";
        std::cout << std::endl;
        std::cout << "radius: ";
        for(int i = 0; i < 4; ++i)
            std::cout << radius[i] << ", ";
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
    }
    */

    auto const closestHit = [&](Ray<vvec3> const ray, RayDistanceRange<vf32> const range) noexcept
        -> Hit
    {
        return std::ranges::fold_left
        (
            vectorizedSpheres | std::views::enumerate
                              | std::views::transform([&](auto const &pair) noexcept
                -> Hit
            {
                auto const &[i, vs] = pair;
                auto const rdr = rayIntersection(ray, vs, range);
                
                vf32 tx4 = range.tFar;
                where(nonempty(rdr), tx4) = hitDistance(rdr);
                f32 const tMin = stdx::hmin(tx4);
                if(tMin == 1.f / 0.f)
                   return nullHit;

                ASSERT(stdx::popcount(tx4 == tMin) == 1, "same spheres in data!");
                u32 const offset = stdx::find_first_set(tx4 == tMin);
                
                return
                {
                    .sphereI = u32(i) * chunkSize + offset,
                    .t = tMin,
                };
            }),
            nullHit,
            [&](Hit const &accum, Hit const &hit) noexcept
            {
                if(miss(accum))
                    return hit;
                if(miss(hit))
                    return accum;
                return (accum.t < hit.t) ? accum : hit;
            }
        );
    };

    auto const trace = [&](Ray<vec3> const ray) noexcept
        -> vec3
    {
        vec3 const   skyColor = {0.53f, 0.81f, 0.92f};
        vec3 const lightColor = {1.00f, 0.98f, 0.88f};
        vec3 const lightDir   = utils::normalize(vec3{3.f, 3.f, -1.f});

        auto const hit = closestHit(to<vf32>(ray), zeroToInf<vf32>);
        if(miss(hit))
            return skyColor;

        auto const [i, t] = hit;
        vec3 const pos = ray.pos + ray.dir * t;
        vec3 const norm = utils::normalize(pos - sphere[i].origin);
        float const NL = std::max(0.f, dot(norm, lightDir));

        auto const shadowHit = closestHit({pos, lightDir}, {1e-3f, 1.f / 0.f});
        return albedo[i] * (skyColor * 0.1f + lightColor * NL * (!miss(shadowHit) ? 0.f : 0.7f));
    };

    std::ofstream file("out.ppm");
    u32 const width  = 1920u;
    u32 const height = 1080u;
    file << "P3\n" << width << ' ' << height << "\n255\n";

    Camera const camera =
    {
        .origin = {0.f, 0.f, -2.f},
        .at = {0.f, 0.f, 1.f},
        .up = {0.f, 1.f, 1.f},
        .fov = 0.55f,
        .aspectRatio = f32(width) / f32(height),
    };

    for(u32 y = 0u; y < height; ++y)
    for(u32 x = 0u; x <  width; ++x)
    {
        f32 const u = -1.f + 2.f * (0.5f + f32(x)) / f32( width);
        f32 const v =  1.f - 2.f * (0.5f + f32(y)) / f32(height);
        auto const [r, g, b] = trace(camera.castRay(u, v));

        auto const encode = +[](f32 const f) noexcept
        {
            f32 const c = f < 0.0031308f
                ? f * 12.92f
                : 1.055f * std::pow(f, 1.f / 2.4f) - 0.055f;
            return u32(std::round(255.f * c));
        };
        file << encode(r) << ' '
             << encode(g) << ' '
             << encode(b) << ' ';
    }
}

