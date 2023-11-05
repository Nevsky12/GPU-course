#pragma once
#include <gltf/gltf.h>
#include <ranges>

namespace gltf::view
{

inline auto buffer(gltf::GLTF const &file) noexcept
{
    return [&file](gltf::Accessor const &accessor, auto const &proj = lambdaE1(x, x)) noexcept
    {
        gltf::BufferView const &bufferView = file.json.bufferViews[accessor.bufferView];
        auto const bytes = file.bytes.subspan(bufferView.byteOffset, bufferView.byteLength)
                                     .subspan(  accessor.byteOffset);
        u32 const stride = bufferView.byteStride
            ? *bufferView.byteStride
            : accessor.sizeofElement();

        assert(accessor.count * stride == bytes.size());
        return std::views::iota(0u, accessor.count) | std::views::transform
        (
            [bytes, stride](u32 const i) noexcept
            {
                return bytes.subspan(i * stride, stride);
            }
        ) | std::views::transform(proj);
    };
}

// useful projections:
inline u32  bytesAsIdx (std::span<u8 const> const b) noexcept
{
    if(b.size() == 1u)
        return *reinterpret_cast<u8  const *>(b.data());
    if(b.size() == 2u)
        return *reinterpret_cast<u16 const *>(b.data());
    if(b.size() == 4u)
        return *reinterpret_cast<u32 const *>(b.data());
    assert(0);
    return 0u;
}
inline u32  bytesAsU32 (std::span<u8 const> const b) noexcept
{
    assert(b.size() == 4u);
    return *reinterpret_cast<u32 const *>(b.data());
}
inline f32  bytesAsF32 (std::span<u8 const> const b) noexcept
{
    assert(b.size() == 4u);
    return *reinterpret_cast<f32 const *>(b.data());
}
inline vec2 bytesAsVec2(std::span<u8 const> const b) noexcept
{
    assert(b.size() ==  8u);
    return *reinterpret_cast<vec2 const *>(b.data());
}
inline vec3 bytesAsVec3(std::span<u8 const> const b) noexcept
{
    assert(b.size() == 12u);
    return *reinterpret_cast<vec3 const *>(b.data());
}
inline vec4 bytesAsVec4(std::span<u8 const> const b) noexcept
{
    assert(b.size() == 16u);
    return *reinterpret_cast<vec4 const *>(b.data());
}
inline mat3 bytesAsMat3(std::span<u8 const> const b) noexcept
{
    assert(b.size() == 36u);
    return *reinterpret_cast<mat3 const *>(b.data());
}
inline mat4 bytesAsMat4(std::span<u8 const> const b) noexcept
{
    assert(b.size() == 64u);
    return *reinterpret_cast<mat4 const *>(b.data());
}

} // namespace gltf::view
