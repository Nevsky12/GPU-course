#pragma once
#include "simd_ray.h"
#include <optional>

struct Sphere
{
    vec3 origin;
    f32 radius;
};

template<typename Vec3>
struct SphereBucket
{
    Vec3 origin;
    typename Vec3::type radius;
};

template<typename Float>
using RDR = RayDistanceRange<Float>;

template<typename Vec3>
inline auto rayIntersection( Ray<Vec3> const ray
                           , SphereBucket<Vec3> const sphere
                           , RDR<typename Vec3::type> const range
                           ) noexcept
{
    Vec3 const s = ray.pos - sphere.origin;
    typename Vec3::type const ds = dot(ray.dir, s);
    typename Vec3::type const d2 = dot(ray.dir, ray.dir);
    typename Vec3::type const det = ds * ds + d2 * (sphere.radius * sphere.radius - dot(s, s));
    RDR const r =
    {
        .tNear = (-ds - stdx::sqrt(det)) / d2,
        .tFar  = (-ds + stdx::sqrt(det)) / d2,
    };
    return RayDistanceRange<typename Vec3::type>
    {
        utils::foldr1(stdx::max<f32, ABI>, vec2x4{r.tNear, range.tNear}),
        utils::foldr1(stdx::min<f32, ABI>, vec2x4{r.tFar , range.tFar }),
    };
}
