#pragma once
#include <gltf/utils/ray.h>
namespace gltf::utils
{

struct Camera
{
    vec3 pos = {0.f, 0.f, 0.f};
    mat3 basis;
    vec3 scale;

    static vec3 scaleFrom(f32 const fov, f32 const aspectRatio) noexcept
    {
        return {std::tan(fov) * aspectRatio, std::tan(fov), 1.f};
    }
    static vec3 atFrom(vec3 const position, f32 const theta, f32 const phi) noexcept
    {
        return position + vec3
        {
            std::cos(theta) * std::cos(phi),
            std::cos(theta) * std::sin(phi),
            std::sin(theta),
        };
    }
    Camera(vec3 const position, vec3 const at, vec3 const up, vec3 const s) noexcept
        : pos(position)
        , scale(s)
    {
        vec3 const z = normalize(position - at);
        vec3 const x = normalize(cross(up, z));
        vec3 const y = cross(z, x);
        basis = {x, y, z};
    }

    Ray castRay(vec2 const &uv) const noexcept
    {
        return Ray
        {
            pos,
            normalize(basis * (scale * vec3(uv, -1.f))),
        };
    }
};

} // namespace gltf::utils
