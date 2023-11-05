#pragma once
#include <gltf/types.h>

namespace gltf
{

struct Accessor
{
    u32 bufferView;
    u32 byteOffset = 0;
    enum class ComponentType : u32
    {
        S8  = 5120u,
        U8  = 5121u,
        S16 = 5122u,
        U16 = 5123u,
        U32 = 5125u,
        F32 = 5126u,
    } componentType;
    bool normalized = false;
    u32 count;
    enum class Type : u32
    {
        Scalar =  1u,
        Vec2   =  2u,
        Vec3   =  3u,
        Vec4   =  4u,
        Mat3   =  9u,
        Mat4   = 16u,
    } type;
    std::vector<f32> max, min;
    std::string_view name;

    u32 sizeofElement() const noexcept
    {
        switch(componentType)
        {
            case ComponentType::S8:  return 1u * u32(type);
            case ComponentType::U8:  return 1u * u32(type);
            case ComponentType::S16: return 2u * u32(type);
            case ComponentType::U16: return 2u * u32(type);
            case ComponentType::U32: return 4u * u32(type);
            case ComponentType::F32: return 4u * u32(type);
            default: assert(0); return 0u;
        }
    }

    Accessor(gltf::json &&) noexcept;
};

} // namespace gltf
