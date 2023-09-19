#include <algorithm>
#include <fstream>
#include <ranges>
#include <variant>
#include <iostream>

#include <utils/camera.h>
#include <utils/aabb.h>
#include <utils/sphere.h>
#include <utils/triangle.h>
#include <utils/sdTorus.h>

template<typename... Ts>
struct OverloadSet: Ts...
{
    using Ts::operator()...;
};

int main()
{
    constexpr unsigned int packSize = 18u;
    std::variant<AABB, Sphere, Triangle, TorusXZ> const geomPack[packSize] =
    {         
        AABB
        {
            .rMin = {-0.3f, 0.4f,  3.f},
            .rMax = { 0.3f, 1.4f, 10.f},
        },  
        AABB
        {
            .rMin = {-0.3f, 0.4f, -2.f},
            .rMax = { 0.3f, 1.4f,  2.f},
        },
        AABB
        {
            .rMin = {-0.3f, 0.4f, -2.5f},
            .rMax = {0.3f, 1.4f,  -4.f},
        },
        AABB
        {
            .rMin = {-0.7f, -1.1f, -20.f},
            .rMax = { 0.7f,  0.3f,  20.f},
        },
        Sphere
        {
            .origin = {0.f, -1000.f, 0.f},
            .radius = 1000.f,
        },
        AABB                                   
        {
            .rMin = {-20.f, 0.f, 6.f},
            .rMax = {-4.f, 20.f, 4.f},
        },
        AABB
        {
            .rMin = {1000.f, -100.f, 6.f},
            .rMax = {1.f, 20.f, 4.f},
        },
        AABB
        {
            .rMin = {-6.f, 2.f, 6.f},
            .rMax = { 3.f, 3.f, 0.5f},
        },
        AABB
        {
            .rMin = {-10.f, 2.f, 6.f},
            .rMax = {3.f, 20.f, 4.f}, 
        },
        AABB
        {
            .rMin = {-0.5f, 0.4f, 3.f},
            .rMax = {0.5f, 1.0f, 10.f},
        },
        AABB
        {
            .rMin = {-0.5f, 0.4f, -2.f},
            .rMax = {0.5f, 1.0f, 2.f},
        },
        AABB
        {
            .rMin = {-0.5f, 0.4f, -2.5f},
            .rMax = {0.5f, 1.0f, -4.0f},
        },
        Triangle
        {
            .r0 = { 0.3f, 1.4f, -4.0f},
            .r1 = { 0.3f, 0.4f, -4.0f},
            .r2 = { 0.0f, 0.7f, -4.3f},
        },
        Triangle
        {
            .r0 = {-0.3f, 1.4f, -4.0f},
            .r1 = {-0.3f, 0.4f, -4.0f},
            .r2 = { 0.0f, 0.7f, -4.3f},
        },
        Triangle                        
        {
            .r0 = { 0.3f, 1.4f, -4.0f},
            .r1 = {-0.3f, 1.4f, -4.0f},
            .r2 = { 0.0f, 0.7f, -4.3f},
        },
        Triangle                         
        {
            .r0 = { 0.3f, 0.4f, -4.0f},
            .r1 = {-0.3f, 0.4f, -4.0f},
            .r2 = { 0.0f, 0.7f, -4.3f},
        },
        TorusXZ
        {
            .O = {5.f, 5.f, 4.f},
            .r = 0.7f,
            .R = 2.0f,
        },
        TorusXZ
        {
            .O = {-5.f, 5.f, 4.f},
            .r = 0.7f,
            .R = 2.0f,
        },
    };
    
    vec3 const gatesWall   = {0.1f, 0.1f, 0.1f};
    vec3 const gatesEnter  = {0.2f, 0.2f, 0.2f};
    vec3 const train       = {0.8f, 0.4f, 0.2f};
    vec3 const trainArmour = {0.2f, 0.2f, 0.3f};
    vec3 const railway     = {0.2f, 0.1f, 0.1f};
    vec3 const ground      = {0.2f, 0.5f, 0.2f};
    vec3 const barebuh     = ground;
    vec3 const albedo[packSize] =
    {
        train,
        train,
        train,
        railway,
        ground,
        gatesWall,
        gatesWall,
        gatesEnter,
        gatesWall,
        trainArmour,
        trainArmour,
        trainArmour,

        trainArmour,
        trainArmour,
        trainArmour,
        trainArmour,
        barebuh,
        barebuh,
    };

    struct Hit
    {
        u32 objI;
        f32 t;
    };
    auto const closestHit = [geomPack](Ray const ray, RayRange const range) noexcept
        -> std::optional<Hit>
    {
        return std::ranges::fold_left
        (
            geomPack | std::views::enumerate              
                     | std::views::transform([=](auto const &pair) noexcept
               {
                    auto const &[i, o] = pair;
                    auto const rsi = std::visit
                    (
                        OverloadSet                                                                              
                        {
                           [ray       ](AABB     const &obj) noexcept {return rayAABBIntersection(ray, obj);},
                           [ray       ](Sphere   const &obj) noexcept {return raySphereIntersection(ray, obj);},
                           [ray       ](Triangle const &obj) noexcept {return rayTriangleIntersection(ray, obj);},
                           [ray, range](TorusXZ  const &obj) noexcept {return raySdTorusIntersection(ray, obj, range);},
                        },
                        o
                    );
                    return nonempty(rsi + range)
                         ? std::optional<Hit>
                         {{
                              .objI = u32(i),
                              .t = contains(range, rsi.tMin) ? rsi.tMin : rsi.tMax
                         }}
                         : std::nullopt;
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
        vec3 const lightDir   = normalize({-3.f, 5.f, -5.f});

        auto const hit = closestHit(ray, zeroToInf);
        if(!hit)
            return skyColor;

        auto const [i, t] = *hit;
        vec3 const pos = ray.origin + ray.direction * t;
        vec3 const norm =
        std::visit
        (
           OverloadSet
           {
              [&](AABB     const &obj) noexcept 
              {
                    auto const &[rMin, rMax] = obj;
                    vec3 const C = (rMin + rMax) / 2;
                    vec3 const d = (rMax - rMin) / 2;
                    vec3 const PC = pos - C;
                    f32 const bias = 1.000001f;
                    vec3 const mod = PC / abs(d) * bias;
                    return normalize
                    ({
                          f32(u32(mod.x)),
                          f32(u32(mod.y)),
                          f32(u32(mod.z)),
                    });
              },
              [&](Sphere   const &obj) noexcept {return normalize(pos - obj.origin);},
              [&](Triangle const &obj) noexcept 
              {
                  auto const &[r0, r1, r2] = obj;
                  return normalize(cross(r1 - r0, r2 - r0));
              },
              [&](TorusXZ const &obj) noexcept
              {
                  f32 const prec = 1e-4f;
                  return normalize
                  ({
                        sdTorus(pos + vec3{prec, 0.f, 0.f}, obj) - sdTorus(pos - vec3{prec, 0.f, 0.f}, obj),
                        sdTorus(pos + vec3{0.f, prec, 0.f}, obj) - sdTorus(pos - vec3{0.f, prec, 0.f}, obj),
                        sdTorus(pos + vec3{0.f, 0.f, prec}, obj) - sdTorus(pos - vec3{0.f, 0.f, prec}, obj),
                  });
              }
           },
           geomPack[i]
        ); 
        
        //vec3 const norm = normalize(pos - gPosI);
        float const NL = std::max(0.f, dot(norm, lightDir));

        auto const shadowHit = closestHit({pos, lightDir}, {1e-3f, 1.f / 0.f});
        return albedo[i] * (skyColor * 0.1f + lightColor * NL * (shadowHit ? 0.01f : 0.6f));
    };

    std::ofstream file("out.ppm");
    u32 const width  = 1920u;
    u32 const height = 1080u;
    file << "P3\n" << width << ' ' << height << "\n255\n";

    Camera const camera =
    {
        .origin = {3.0f, 1.6f, -4.5f},
        .at     = {1.8f, 1.0f,  -1.f},
        .up     = {0.f, 1.2f,  0.f},
        .fov    = 0.88f,
        .aspectRatio = f32(width) / f32(height),
    };

    for(u32 y = 0u; y < height; ++y)
    for(u32 x = 0u; x <  width; ++x)
    {
        f32 const u = -1.f + 2.f * (0.5f + f32(x)) / f32( width);
        f32 const v =  1.f - 2.f * (0.5f + f32(y)) / f32(height);
        auto const [r, g, b] = trace(camera.castRay(u, v));

        auto const encode = +[](f32 const grad) noexcept
        {
            f32 const f = grad < 0.0031308f
                ? grad * 12.92f
                : 1.055f * std::pow(grad, 1.f / 2.4f) - 0.055f;
            return u32(std::round(255.f * f));
        };
        file << encode(r) << ' '
             << encode(g) << ' '
             << encode(b) << ' ';
    }
}
