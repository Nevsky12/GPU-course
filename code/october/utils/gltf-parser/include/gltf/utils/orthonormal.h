#pragma once
#include <gltf/types.h>
namespace gltf::utils
{

inline mat3 orthonormal(vec3 const n) noexcept
    // transpose(orthonormal(n)) * n = {0, 0, 1}
{
    // https://jcgt.org/published/0006/01/01/
    f32 const sign = n.z >= 0.f ? 1.f : -1.f;
    f32 const a = -1.f / (sign + n.z);
    vec3 const b =
    {
        a * n.x * n.x,
        a * n.x * n.y,
        a * n.y * n.y,
    };
    return
    {
        {1.f + sign * b.x, sign * b.y, -sign * n.x},
        {b.y, sign + b.z, -n.y},
        n,
    };
}

} // namespace gltf::utils
