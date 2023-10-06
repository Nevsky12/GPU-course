#include <algorithm>
#include <fstream>
#include <numeric>
#include <ranges>

#include <utils/bvh.h>
#include <utils/camera.h>
#include <utils/triangle.h>
#include <utils/wfobj.h>

int main()
{
    std::fstream in(RES_DIR "A6M.obj");
    auto const toBox = [](Triangle const &t) noexcept
    {
        return AABB
        {
            min(t.r0, min(t.r1, t.r2)),
            max(t.r0, max(t.r1, t.r2)),
        };
    };

    struct Hit
    {
        vec3 pos;
        vec3 norm;
    };
    auto const closestHit = [bvh = createBVH(parseOBJ(in), toBox)](Ray const ray, RayRange const range) noexcept
        -> std::optional<Hit>
    {
        return rayIntersection(ray, bvh, range).transform
        (
            [](auto const &pair) noexcept
            {
                auto const &[tri, rti] = pair;
                vec3 const r[3] = {tri.r0, tri.r1, tri.r2};
                return Hit
                {
                    .pos  = interpolate(r, rti.barycentrics),
                    .norm = normalize(cross(r[1] - r[0], r[2] - r[0])),
                };
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

        auto const [pos, norm] = *hit;
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
