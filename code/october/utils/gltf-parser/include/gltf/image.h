#pragma once
#include <gltf/types.h>

namespace gltf
{

struct Image
{
    std::optional<std::string_view> mimeType;
    u32 bufferView;
    std::optional<std::string_view> name;

    Image(gltf::json &&) noexcept;
};

} // namespace gltf
