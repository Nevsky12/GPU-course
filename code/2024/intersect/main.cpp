#include <algorithm>
#include <iostream>
#include <fstream>
#include <ranges>

#include <utils/simd_ray.h>
#include <utils/camera.h>
#include <utils/sphere.h>
#include <utils/overloaded.h>


int main()
{
    std::vector<Sphere> const sphere =
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
    std::vector<vec3> const albedo =
    {
        {0.8f, 0.3f, 0.3f},
        {0.3f, 0.3f, 0.8f},
        {0.8f, 0.8f, 0.8f},
        {0.2f, 0.5f, 0.2f},
    };
    assert(sphere.size() <= albedo.size());

    using vvec3 = vec3x4;
    using vf32  =  f32x4;
    u32 const regSize = f32x4::size();

    struct Hitx4
    {
        u32x4 I;
        f32x4 t;
    };
    
    Hitx4 const nullHitx4 = 
    {
        -1u,
        +1.f / 0.f,
    };

    struct Hit
    {
        u32 I;
        f32 t;
    };

    Hit const nullHit = 
    {
        -1u,
        +1.f / 0.f,
    };

    std::vector<u32>  globalIds(std::ranges::size(sphere));
    std::ranges::iota(globalIds, 0);
    std::vector<vec3> const origins = to_vector(sphere | &Sphere::origin);
    std::vector<f32 > const radiuss = to_vector(sphere | &Sphere::radius);

    auto const isMissed = []<typename Unsigned>(Unsigned const j) noexcept
    {
        return j == -1u;
    };

    auto const closestHit = [&](Ray<vvec3> const ray, RayDistanceRange<vf32> const range) noexcept
                            -> Hit
    {
        return std::ranges::fold_left
        ( 
            std::ranges::zip_view
            (
                globalIds | std::views::chunk(regSize),
                origins   | std::views::chunk(regSize),
                radiuss   | std::views::chunk(regSize)
            )
            | std::views::transform([&](auto const &triada) noexcept
            {
                alignas(stdx::memory_alignment_v<u32x4>) auto const ids     = std::get<0>(triada);
                alignas(stdx::memory_alignment_v<vf32 >) auto const radiuss = std::get<2>(triada);

                std::vector<vec3> const origins = to_vector(std::get<1>(triada));
                alignas(stdx::memory_alignment_v<vf32>) std::vector<f32> const originsX = to_vector(origins | &vec3::x);
                alignas(stdx::memory_alignment_v<vf32>) std::vector<f32> const originsY = to_vector(origins | &vec3::y);
                alignas(stdx::memory_alignment_v<vf32>) std::vector<f32> const originsZ = to_vector(origins | &vec3::z);

                assert(std::ranges::size(ids    ) == regSize);
                assert(std::ranges::size(origins) == regSize);
                assert(std::ranges::size(radiuss) == regSize);

                auto const hit = rayIntersection
                (
                    ray, 
                    SphereBucket<vvec3>
                    (
                        vvec3
                        (
                            vf32(&originsX[0], stdx::vector_aligned),
                            vf32(&originsY[0], stdx::vector_aligned),
                            vf32(&originsZ[0], stdx::vector_aligned)
                        ), 
                        vf32(&radiuss[0], stdx::vector_aligned)
                    ), 
                    range
                );
                
                Hitx4 monsterHit = nullHitx4;
                where(nonempty(hit), monsterHit.I) = u32x4(&ids[0], stdx::vector_aligned);
                where(nonempty(hit), monsterHit.t) = hitDistance(hit);
                return monsterHit;
            }),
            nullHit,
            [&](Hit const &accum, Hitx4 const &hit) noexcept
               -> Hit
            {
                f32 const tMin = stdx::hmin(hit.t);
                u32 const uMin = stdx::find_first_set(tMin == hit.t);

                /*
                std::cout << std::endl;
                std::cout << std::endl;
                std::cout << "Accum: " << accum.I << "; " << accum.t << std::endl;
                std::cout << "[uMin; tMin] --> " << uMin << "; " << tMin << std::endl;
                */

                if(isMissed(accum.I))
                   return {uMin, tMin};
                if(isMissed(uMin))
                   return accum;

                return (accum.t < tMin) 
                     ? accum
                     : Hit{uMin, tMin};
            }
        );
    };

    auto const trace = [&](Ray<vec3> const ray) noexcept
         -> vec3
    {
        vec3 const   skyColor = {0.53f, 0.81f, 0.92f};
        vec3 const lightColor = {1.00f, 0.98f, 0.88f};
        vec3 const lightDir   = utils::normalize(vec3{3.f, 3.f, -1.f});

        auto const hit = closestHit
        (
            to<vf32>(ray), 
            zeroToInf<vf32>
        );

        if(isMissed(hit.I))
            return skyColor;

        auto const [i, t] = hit;
        vec3 const pos = ray.pos + ray.dir * t;
        vec3 const norm = normalize(pos - sphere[i].origin);
        float const NL = std::max(0.f, dot(norm, lightDir));

        auto const shadowHit = closestHit({pos, lightDir}, {2e-4f, 1.f / 0.f});
        return albedo[i] * (skyColor * 0.1f + lightColor * NL * (isMissed(shadowHit.I) ? 0.f : 0.7f));
    };

    std::ofstream file("out.ppm");
    u32 const width  = 1920u;
    u32 const height = 1080u;
    file << "P3\n" << width << ' ' << height << "\n255\n";

    Camera const camera =
    {
        .origin = {0.f, 0.8f, -2.f},
        .at = {0.f, 0.f, 0.f},
        .up = {0.f, 1.f, 0.f},
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

