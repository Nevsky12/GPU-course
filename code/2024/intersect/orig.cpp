#include <algorithm>
#include <fstream>
#include <ranges>
#include <iostream>
#include <utils/camera.h>
#include <utils/sphere.h>

int main()
{
    /*
    Sphere<vec3> const sphere[4] =
    {
        {
            .origin = {0.f, 0.1f, 0.f},
            .radius = 0.5f,
        },
        {
            .origin = {0.f, 0.7f, 0.f},
            .radius = 0.3f,
        },
        {
            .origin = {0.f, -0.4f, 0.f},
            .radius = 0.7f,
        },
        {
            .origin = {0.f, -1000.f, 0.f},
            .radius = 999.5f,
        },
    };
    vec3 const albedo[4] =
    {
        {0.8f, 0.3f, 0.3f},
        {0.3f, 0.3f, 0.8f},
        {0.8f, 0.8f, 0.8f},
        {0.2f, 0.5f, 0.2f},
    };
    */

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

    struct Hit
    {
        u32 sphereI;
        f32 t;
    };

    Hit const nullHit = {-1u, +1.f / 0.f};

    auto const miss = [](Hit const h) { return h.sphereI == -1u; };

    auto const closestHit = [&](Ray<vec3> const ray, RayDistanceRange<f32> const range) noexcept
        -> Hit
    {
        return std::ranges::fold_left
        (
            sphere | std::views::enumerate
                   | std::views::transform([&](auto const &pair) noexcept
                -> Hit
            {
                auto const &[i, s] = pair;
                auto const rdr =  rayIntersection(ray, s, range);
                if(empty(rdr))
                   return nullHit;

                return 
                {
                    .sphereI = u32(i),
                    .t = hitDistance(rdr),
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

        auto const hit = closestHit(ray, zeroToInf<f32>);
        if(miss(hit))
            return skyColor;

        auto const [i, t] = hit;
        vec3 const pos = ray.pos + ray.dir * t;
        vec3 const norm = utils::normalize(pos - sphere[i].origin);
        float const NL = std::max(0.f, dot(norm, lightDir));

        auto const shadowHit = closestHit({pos, lightDir}, {2e-4f, 1.f / 0.f});
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

