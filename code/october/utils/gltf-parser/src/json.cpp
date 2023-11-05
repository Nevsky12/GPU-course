#include <gltf/json.h>
#include "common.h"

namespace gltf
{

JSON::JSON(std::vector<u8> const &json) noexcept
{
    simdjson::ondemand::parser parser;
    simdjson::ondemand::document doc = parser.iterate(json.data(), json.size(), json.capacity());

    for(auto field : doc.get_object())
    {
        std::string_view const key = field.unescaped_key();
        simdjson::ondemand::value value = field.value();

        FILL_FIELD(key, value, accessors    , valueToVector(valueTo<gltf::Accessor>));
        FILL_FIELD(key, value, asset        , valueTo<gltf::Asset>);
        FILL_FIELD(key, value, buffers      , valueToVector(valueTo<gltf::Buffer>));
        FILL_FIELD(key, value, bufferViews  , valueToVector(valueTo<gltf::BufferView>));
        FILL_FIELD(key, value, images       , valueToVector(valueTo<gltf::Image>));
        FILL_FIELD(key, value, materials    , valueToVector(valueTo<gltf::Material>));
        FILL_FIELD(key, value, meshes       , valueToVector(valueTo<gltf::Mesh>));
        FILL_FIELD(key, value, nodes        , valueToVector(valueTo<gltf::Node>));
        FILL_FIELD(key, value, samplers     , valueToVector(valueTo<gltf::Sampler>));
        FILL_FIELD(key, value, scene        , valueToU32);
        FILL_FIELD(key, value, scenes       , valueToVector(valueTo<gltf::Scene>));
        FILL_FIELD(key, value, textures     , valueToVector(valueTo<gltf::Texture>));
    }
}

} // namespace gltf
