#include <algorithm>
#include <fstream>
#include <numeric>
#include <ranges>

#include <utils/camera.h>
#include <utils/triangle.h>
#include <utils/wfobj.h>

int main()
{
    std::fstream in(RES_DIR "A6M.obj");
    std::vector<Triangle> const triangle = parseOBJ(in);

    struct Hit
    {
        u32 i;
        f32 t;
    };
    auto const closestHit = [triangle](Ray const ray, RayRange const range) noexcept
        -> std::optional<Hit>
    {
        return std::ranges::fold_left
        (
            triangle | std::views::enumerate
                     | std::views::transform([=](auto const &pair) noexcept
                -> std::optional<Hit>
            {
                auto const &[i, tri] = pair;
                return rayIntersection(ray, tri, range).transform
                (
                    [i](RayTriangleIntersection const &rti) noexcept
                    {
                        return Hit
                        {
                            .i = u32(i),
                            .t = rti.t ,
                        };
                    }
                );
            }),
            std::nullopt,
            [](std::optional<Hit> const &accum, std::optional<Hit> const &hit) noexcept
            {
                if(!accum)
                    return hit;
                if(!hit)
                    return accum;
                return (accum->t < hit->t) ? accum : hit;
            }
        );
    };

    auto const trace = [&](Ray const ray) noexcept
        -> vec3
    {
        vec3 const   skyColor = {0.53f, 0.81f, 0.92f};
        vec3 const lightColor = {1.00f, 0.98f, 0.88f};
        vec3 const lightDir   = normalize({3.f, 3.f, -1.f});

        auto const hit = closestHit(ray, zeroToInf);
        if(!hit)
            return skyColor;

        auto const [i, t] = *hit;
        vec3 const pos = ray.origin + ray.direction * t;
        auto const [r0, r1, r2] = triangle[i];
        vec3 const norm = normalize(cross(r1 - r0, r2 - r0));
        float const NL = std::max(0.f, dot(norm, lightDir));

        auto const shadowHit = closestHit({pos, lightDir}, {2e-4f, 1.f / 0.f});
        vec3 const albedo = {0.8f, 0.3f, 0.3f};
        return albedo * (skyColor * 0.1f + lightColor * NL * (shadowHit ? 0.f : 0.7f));
    };

    std::ofstream file("out.ppm");
    u32 const width  = 1920u;
    u32 const height = 1080u;
    file << "P3\n" << width << ' ' << height << "\n255\n";

    Camera const camera =
    {
        .origin = {5.f, 3.f, 10.f},
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
