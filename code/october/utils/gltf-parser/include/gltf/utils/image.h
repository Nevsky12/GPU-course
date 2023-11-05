#pragma once
#include <memory>
#include <gltf/gltf.h>

namespace gltf::utils
{

template<typename T>
class Image
{
    std::unique_ptr<T[], void(*)(void *)> data;
    i32 width, height, components;

public:
    Image(gltf::GLTF const &, gltf::Image const &) noexcept;

    ivec2         extent(             ) const noexcept {return ivec2(width, height);}
    gvec<T, 4>    fetch (ivec2 const p) const noexcept
    {
        T const * const ptr = data.get() + (width * p.y + p.x) * components;
        switch(components)
        {
            case 1: return {ptr[0],   T(0),   T(0),   T(0)};
            case 2: return {ptr[0], ptr[1],   T(0),   T(0)};
            case 3: return {ptr[0], ptr[1], ptr[2],   T(0)};
            case 4: return {ptr[0], ptr[1], ptr[2], ptr[3]};
            default: assert(0); return {};
        }
    }
};

} // namespace gltf::utils
