#include <gltf/utils/image.h>
#include <gltf/buffer_view.h>

extern "C"
{
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
}

namespace gltf::utils
{

template<>
Image<u8 >::Image(gltf::GLTF const &gltf, gltf::Image const &image) noexcept
    : data(nullptr, nullptr)
{
    gltf::BufferView const &bufferView = gltf.json.bufferViews[image.bufferView];
    std::span<gltf::u8 const> const bytes = gltf.bytes.subspan(bufferView.byteOffset, bufferView.byteLength);
    data = {stbi_load_from_memory (bytes.data(), int(bytes.size()), &width, &height, &components, 0), stbi_image_free};
}

template<>
Image<f32>::Image(gltf::GLTF const &gltf, gltf::Image const &image) noexcept
    : data(nullptr, nullptr)
{
    gltf::BufferView const &bufferView = gltf.json.bufferViews[image.bufferView];
    std::span<gltf::u8 const> const bytes = gltf.bytes.subspan(bufferView.byteOffset, bufferView.byteLength);
    data = {stbi_loadf_from_memory(bytes.data(), int(bytes.size()), &width, &height, &components, 0), stbi_image_free};
}

} // namespace gltf::utils
