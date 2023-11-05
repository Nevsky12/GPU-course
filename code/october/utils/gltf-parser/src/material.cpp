#include <gltf/material.h>
#include "common.h"

namespace gltf
{

Material::TextureInfo::TextureInfo(gltf::json &&object) noexcept
{
    for(auto field : object)
    {
        std::string_view const key = field.unescaped_key();
        simdjson::ondemand::value value = field.value();

        FILL_FIELD(key, value, index    , valueToU32);
        FILL_FIELD(key, value, texCoord , valueToU32);
    }
}
Material::NormalTextureInfo::NormalTextureInfo(gltf::json &&object) noexcept
{
    for(auto field : object)
    {
        std::string_view const key = field.unescaped_key();
        simdjson::ondemand::value value = field.value();

        FILL_FIELD(key, value, index    , valueToU32);
        FILL_FIELD(key, value, texCoord , valueToU32);
        FILL_FIELD(key, value, scale    , valueToF32);
    }
}
Material::OcclusionTextureInfo::OcclusionTextureInfo(gltf::json &&object) noexcept
{
    for(auto field : object)
    {
        std::string_view const key = field.unescaped_key();
        simdjson::ondemand::value value = field.value();

        FILL_FIELD(key, value, index    , valueToU32);
        FILL_FIELD(key, value, texCoord , valueToU32);
        FILL_FIELD(key, value, strength , valueToF32);
    }
}
Material::Extensions::Extensions(gltf::json &&object) noexcept
{
    for(auto field : object)
    {
        std::string_view const key = field.unescaped_key();
        simdjson::ondemand::value value = field.value();

        FILL_FIELD(key, value, KHR_materials_emissive_strength, valueTo<Material::Extensions::KHR_MaterialsEmissiveStrength>);
    }
}
Material::Extensions::KHR_MaterialsEmissiveStrength::KHR_MaterialsEmissiveStrength(gltf::json &&object) noexcept
{
    for(auto field : object)
    {
        std::string_view const key = field.unescaped_key();
        simdjson::ondemand::value value = field.value();

        FILL_FIELD(key, value, emissiveStrength, valueToF32);
    }
}
Material::PBRMetallicRoughness::PBRMetallicRoughness(gltf::json &&object) noexcept
{
    for(auto field : object)
    {
        std::string_view const key = field.unescaped_key();
        simdjson::ondemand::value value = field.value();

        FILL_FIELD(key, value, baseColorFactor          , valueToVec4);
        FILL_FIELD(key, value, baseColorTexture         , valueTo<TextureInfo>);
        FILL_FIELD(key, value, metallicFactor           , valueToF32);
        FILL_FIELD(key, value, roughnessFactor          , valueToF32);
        FILL_FIELD(key, value, metallicRoughnessTexture , valueTo<TextureInfo>);
    }
}
Material::Material(gltf::json &&object) noexcept
{
    auto const valueToAlphaMode = [](simdjson::ondemand::value value) noexcept
    {
        std::string_view const sv = {value};
        if(sv == "MASK")
            return AlphaMode::Mask;
        if(sv == "BLEND")
            return AlphaMode::Blend;
        return AlphaMode::Opaque;
    };
    for(auto field : object)
    {
        std::string_view const key = field.unescaped_key();
        simdjson::ondemand::value value = field.value();

        FILL_FIELD(key, value, name                 , valueTo<std::string_view>);
        FILL_FIELD(key, value, pbrMetallicRoughness , valueTo<PBRMetallicRoughness>);
        FILL_FIELD(key, value, normalTexture        , valueTo<NormalTextureInfo>);
        FILL_FIELD(key, value, occlusionTexture     , valueTo<OcclusionTextureInfo>);
        FILL_FIELD(key, value, emissiveTexture      , valueTo<TextureInfo>);
        FILL_FIELD(key, value, emissiveFactor       , valueToVec3);
        FILL_FIELD(key, value, alphaMode            , valueToAlphaMode);
        FILL_FIELD(key, value, alphaCutoff          , valueToF32);
        FILL_FIELD(key, value, doubleSided          , valueTo<bool>);
        FILL_FIELD(key, value, extensions           , valueTo<Material::Extensions>);
    }
}

} // namespace gltf
