#pragma once
#include <gltf/types.h>

namespace gltf
{

struct Texture
{
    std::optional<u32> sampler;
    u32 source;
    std::string_view name;

    Texture(gltf::json &&) noexcept;
};

} // namespace gltf
