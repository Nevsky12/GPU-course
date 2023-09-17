#pragma once
#include "ray.h"

struct TorusXZ
{
    vec3 O;
    f32 r, R;
};

inline f32 sdTorus(vec3 const p, TorusXZ const torus) noexcept
{
   auto const &[O, r, R] = torus;
   vec2 const q = vec2{length(vec3{p.x - O.x, 0.f, p.z - O.z}) - R, p.y - O.y};
   return std::sqrt(q.x * q.x + q.y * q.y) - r; 
}

inline RayRange raySdTorusIntersection( Ray     const ray
                                      , TorusXZ const torus
                                      ) noexcept
{
   auto const &[O, d] = ray;

   f32 const maxDist = 100.f;

   auto const raymarch = [&](vec3 const pos) noexcept -> f32
   {
       f32 dist = 0; 
       auto const raymarchImpl = [&](auto const self, vec3 const p) noexcept -> f32
       {
           f32 const latest = sdTorus(p, torus);
           dist += latest;
		   return (latest > 1e-4f && dist < maxDist)
		       ? self(self, O + d * dist)
		       : dist;
       };
       return raymarchImpl(raymarchImpl, pos);
   };

   f32 const t = raymarch(O);
   f32 const res = t < maxDist
                 ? t
                 : -1.f;

   return {res, res};
}
