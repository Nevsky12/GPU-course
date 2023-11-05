#pragma once
#include <gltf/types.h>

namespace gltf
{

struct Sampler
{
    enum class MagFilter
    {
        Nearest = 9728,
        Linear  = 9729,
    };
    enum class MinFilter
    {
        Nearest = 9728,
        Linear  = 9729,
        NearestMipmapNearest = 9984,
         LinearMipmapNearest = 9985,
        NearestMipmapLinear  = 9986,
         LinearMipmapLinear  = 9987,
    };
    enum class Wrap
    {
        ClampToEdge = 33071,
        MirroredRepeat = 33648,
        Repeat = 10497,
    };

    std::optional<MagFilter> magFilter;
    std::optional<MinFilter> minFilter;
    Wrap wrapS = Wrap::Repeat;
    Wrap wrapT = Wrap::Repeat;
    std::string_view name;

    Sampler(gltf::json &&) noexcept;
};

} // namespace gltf
