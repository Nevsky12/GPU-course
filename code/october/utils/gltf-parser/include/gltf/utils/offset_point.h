#pragma once
#include <gltf/types.h>
namespace gltf::utils
{

inline vec3 offsetPoint(vec3 const pos, vec3 const norm) noexcept
{
    float const intScale = 256.f;
    float const floatScale = 1.f / 65536.f;
    float const origin = 1.f / 32.f;

    vec3 const n = norm * intScale;
    ivec3 const of = ivec3(int(n.x), int(n.y), int(n.z));
    vec3 const p = vec3
    (
        std::bit_cast<float>(std::bit_cast<int>(pos.x) + (pos.x < 0.f ? -of[0] : of[0])),
        std::bit_cast<float>(std::bit_cast<int>(pos.y) + (pos.y < 0.f ? -of[1] : of[1])),
        std::bit_cast<float>(std::bit_cast<int>(pos.z) + (pos.z < 0.f ? -of[2] : of[2]))
    );
    return vec3
    (
        abs(pos.x) < origin ? pos.x + floatScale * n.x : p.x,
        abs(pos.y) < origin ? pos.y + floatScale * n.y : p.y,
        abs(pos.z) < origin ? pos.z + floatScale * n.z : p.z
    );
}

} // namespace gltf::utils
