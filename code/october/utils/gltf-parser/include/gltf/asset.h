#pragma once
#include <gltf/types.h>

namespace gltf
{

struct Asset
{
    std::string_view copyright;
    std::string_view generator;
    std::string_view version;
    std::string_view minVersion;

    Asset(gltf::json &&) noexcept;
};

} // namespace gltf
