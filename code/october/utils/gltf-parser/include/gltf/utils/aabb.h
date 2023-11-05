#pragma once
#include <gltf/utils/ray.h>
namespace gltf::utils
{

using AABB = gvec<vec3, 2>;

inline constexpr RayDistanceRange rayAABBIntersection( const Ray ray
                                                     , const AABB box
                                                     , const RayDistanceRange rdr
                                                     ) noexcept
{
    vec3 const a = (box[0] - ray.pos) / ray.dir;
    vec3 const b = (box[1] - ray.pos) / ray.dir;

    // https://jcgt.org/published/0002/02/02/
    f32 const twoulp = 1.00000024f;
    return
    {
        foldr1(gltf::max<f32>, vec4{gltf::min(a, b), rdr.tNear}),
        foldr1(gltf::min<f32>, vec4{gltf::max(a, b), rdr.tFar }) * twoulp,
    };
}

} // namespace gltf::utils
