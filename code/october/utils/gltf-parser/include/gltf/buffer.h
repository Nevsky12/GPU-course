#pragma once
#include <gltf/types.h>

namespace gltf
{

struct Buffer
{
    u32 byteLength;
    std::string_view name;

    Buffer(gltf::json &&) noexcept;
};

} // namespace gltf
