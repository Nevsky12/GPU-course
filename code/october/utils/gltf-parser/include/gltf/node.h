#pragma once
#include <gltf/types.h>

namespace gltf
{

struct Node
{
    std::optional<u32> mesh;
    std::vector<u32> children;
    std::optional<std::string_view> name;

    mat4 matrix = identity<f32, 4>;
    vec4 rotation = {0.f, 0.f, 0.f, 1.f};
    vec3 scale = {1.f, 1.f, 1.f};
    vec3 translation = {0.f, 0.f, 0.f};

    mat4 transform() const noexcept;

    Node(gltf::json &&) noexcept;
};

} // namespace gltf
