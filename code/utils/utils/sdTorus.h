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
                                      , RayRange const range
                                      ) noexcept
{
   auto const &[O, d] = ray;
   auto const [tMin, tMax] = range;

   f32 t = tMin;
   while(t < tMax)
   {
       f32 const dist = sdTorus(O + d * t, torus);
       if (dist < 1e-5f)
           return {t, t};
       t += dist;
   }
   return {0.f, -1.f / 0.f};
   /*
   return [&](RayRange const rangeT) noexcept -> RayRange
   {
       auto const [tMin, tMax] = rangeT;
       f32 t = tMin;
       auto const raymarchImpl = [&](auto const self) noexcept -> RayRange
       {
           f32 const dist = sdTorus(O + d * t, torus);
           if (dist < 1e-5f)
               return RayRange{t, t};
           t += dist;
		   return (t < tMax)
		       ? self(self)
		       : RayRange{0.f, -1.f / 0.f};
       };
       return raymarchImpl(raymarchImpl);
   }(range);
   */
}
