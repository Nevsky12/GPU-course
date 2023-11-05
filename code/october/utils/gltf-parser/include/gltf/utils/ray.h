#pragma once
#include <gltf/types.h>

namespace gltf::utils
{

struct Ray
{
    vec3 pos;
    vec3 dir;
};

struct RayDistanceRange
{
    f32 tNear;
    f32 tFar;
};
inline constexpr bool contains(RayDistanceRange const rdr, float const t) noexcept {return rdr.tNear <= t && t <= rdr.tFar;}
inline constexpr bool nonempty(RayDistanceRange const rdr               ) noexcept {return rdr.tNear <= rdr.tFar;}

} // namespace gltf::utils
