#pragma once
#include "simd_types.h"

template<typename Vec3>
struct Ray
{
    Vec3 pos, dir;
};

using namespace utils;

template<typename SIMD, typename Basic>
gvec<SIMD, 3> to(gvec<Basic, 3> const &v) noexcept
{
    return 
    {
        SIMD(v.x),
        SIMD(v.y),
        SIMD(v.z),
    };
}

template<typename SIMD, typename Basic>
Ray<gvec<SIMD, 3>> to(Ray<gvec<Basic, 3>> const &bray) noexcept
{
    return
    {
        .pos = to<SIMD>(bray.pos),
        .dir = to<SIMD>(bray.dir),
    };
}

template<typename Float>
struct RayDistanceRange
{
    Float tNear, tFar;
};

template<typename SIMD, typename Basic>
RayDistanceRange<SIMD> to(RayDistanceRange<Basic> const &brdr) noexcept
{
    return
    {
        .tNear = SIMD(brdr.tNear),
        .tFar  = SIMD(brdr.tFar ),
    };
}

template<typename Float>
inline RayDistanceRange<Float> const zeroToInf = {Float(0), Float(1) / Float(0)};

template<typename Float>
inline auto nonempty(RayDistanceRange<Float> const r) noexcept 
       -> decltype
          ( 
              std::declval<Float>() 
           <= 
              std::declval<Float>()
          ) {return r.tNear <= r.tFar;}


template<typename Float>
inline auto    empty(RayDistanceRange<Float> const r) noexcept 
       -> decltype
          (
              nonempty(std::declval<RayDistanceRange<Float>>())
          ) {return !nonempty(r);}


template<typename Float>
inline auto contains(RayDistanceRange<Float> const r, Float const t) noexcept 
       -> decltype
          (
              nonempty(std::declval<RayDistanceRange<Float>>())
          ) {return r.tNear <= t && t <= r.tFar;}


template<typename Float>
inline RayDistanceRange<Float> operator+(RayDistanceRange<Float> const r1, RayDistanceRange<Float> const r2) noexcept
{
    if constexpr(std::is_same_v<Float, f32x4>)
    {
        RayDistanceRange<Float> res = r1;
        where(r1.tNear <  r2.tNear, res.tNear) = r2.tNear;
        where(r2.tFar  <= r1.tFar , res.tFar ) = r2.tFar ;

        return res;
    }
    else return
    {
        r1.tNear < r2.tNear ? r2.tNear : r1.tNear,
        r1.tFar  < r2.tFar  ? r1.tFar  : r2.tFar,
    };
}

template<typename Float>
inline Float hitDistance(RayDistanceRange<Float> const r) noexcept {return r.tNear;}

