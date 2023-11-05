#pragma once
#include <gltf/types.h>

namespace gltf
{

struct BufferView
{
    u32 buffer;
    u32 byteOffset;
    u32 byteLength;
    std::optional<u32> byteStride;

    BufferView(gltf::json &&) noexcept;
};

} // namespace gltf
