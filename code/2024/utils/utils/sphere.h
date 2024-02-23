#pragma once
#include "simd_ray.h"

template<typename Vec3>
struct Sphere
{
    Vec3 origin;
    typename Vec3::type radius;
};

template<typename Float>
using RDR = RayDistanceRange<Float>;

template<typename Vec3>
inline auto rayIntersection( Ray<Vec3> const ray
                           , Sphere<Vec3> const sphere
                           , RDR<typename Vec3::type> const range
                           ) noexcept
{
    Vec3 const s = ray.pos - sphere.origin;
    typename Vec3::type const ds = dot(ray.dir, s);
    typename Vec3::type const d2 = dot(ray.dir, ray.dir);
    typename Vec3::type const det = ds * ds + d2 * (sphere.radius * sphere.radius - dot(s, s));
    if constexpr(std::is_same_v<Vec3, vec3x4>)
    {
        RDR res = {range.tFar, range.tFar};
        where(det >= 0, res.tNear) = stdx::max<f32, ABI>
        (
            (-ds - stdx::sqrt(det)) / d2, 
            range.tNear
        );
        where(det >= 0, res.tFar ) = stdx::min<f32, ABI>
        (
            (-ds + stdx::sqrt(det)) / d2, 
            range.tFar
        );
        
        return res;
    }
    else
    {
        RDR const r = 
        {
            .tNear = (-ds - std::sqrt(det)) / d2,
            .tFar  = (-ds + std::sqrt(det)) / d2, 
        };
        return RDR
        {
            std::max(r.tNear, range.tNear),
            std::min(r.tFar , range.tFar ),
        };
    }
}
