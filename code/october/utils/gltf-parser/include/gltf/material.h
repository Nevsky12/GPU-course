#pragma once
#include <gltf/types.h>

namespace gltf
{

struct Material
{
    struct TextureInfo
    {
        u32 index;
        u32 texCoord = 0u;

        TextureInfo(gltf::json &&) noexcept;
    };
    struct NormalTextureInfo
    {
        u32 index;
        u32 texCoord = 0u;
        f32 scale = 1.f;

        NormalTextureInfo(gltf::json &&) noexcept;
    };
    struct OcclusionTextureInfo
    {
        u32 index;
        u32 texCoord = 0u;
        f32 strength = 1.f;

        OcclusionTextureInfo(gltf::json &&) noexcept;
    };
    struct PBRMetallicRoughness
    {
        vec4 baseColorFactor = {1.f, 1.f, 1.f, 1.f};
        std::optional<TextureInfo> baseColorTexture;
        f32 metallicFactor = 1.f;
        f32 roughnessFactor = 1.f;
        std::optional<TextureInfo> metallicRoughnessTexture;

        PBRMetallicRoughness(gltf::json &&) noexcept;
    };
    struct Extensions
    {
        struct KHR_MaterialsEmissiveStrength
        {
            f32 emissiveStrength;

            KHR_MaterialsEmissiveStrength(gltf::json &&) noexcept;
        };
        std::optional<KHR_MaterialsEmissiveStrength> KHR_materials_emissive_strength;

        Extensions(gltf::json &&) noexcept;
    };

    std::string_view name;
    std::optional<PBRMetallicRoughness> pbrMetallicRoughness;
    std::optional<NormalTextureInfo> normalTexture;
    std::optional<OcclusionTextureInfo> occlusionTexture;
    std::optional<TextureInfo> emissiveTexture;
    vec3 emissiveFactor = {0.f, 0.f, 0.f};
    std::optional<Extensions> extensions;

    enum class AlphaMode : u32
    {
        Opaque  = 0u,
        Mask    = 1u,
        Blend   = 2u,
    } alphaMode = AlphaMode::Opaque;
    f32 alphaCutoff = 0.5f;
    bool doubleSided = false;

    Material(gltf::json &&) noexcept;
};

} // namespace gltf
