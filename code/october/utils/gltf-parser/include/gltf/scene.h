#pragma once
#include <gltf/types.h>

namespace gltf
{

struct Scene
{
    std::vector<u32> nodes;
    std::string_view name;

    Scene(gltf::json &&) noexcept;
};

} // namespace gltf
