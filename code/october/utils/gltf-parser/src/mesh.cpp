#include <gltf/mesh.h>
#include "common.h"

namespace gltf
{

Mesh::Primitive::Attributes::Attributes(gltf::json &&object) noexcept
{
    for(auto field : object)
    {
        std::string_view const key = field.unescaped_key();
        simdjson::ondemand::value const value = field.value();

        FILL_FIELD(key, value, POSITION     , valueToU32);
        FILL_FIELD(key, value, TEXCOORD_0   , valueToU32);
        FILL_FIELD(key, value, NORMAL       , valueToU32);
        FILL_FIELD(key, value, TANGENT      , valueToU32);
    }
}
Mesh::Primitive::Primitive(gltf::json &&object) noexcept
{
    for(auto field : object)
    {
        std::string_view const key = field.unescaped_key();
        simdjson::ondemand::value const value = field.value();

        FILL_FIELD(key, value, attributes   , valueTo<Attributes>);
        FILL_FIELD(key, value, indices      , valueToU32);
        FILL_FIELD(key, value, material     , valueToU32);
        FILL_FIELD(key, value, mode         , valueToEnum<Mode>);
    }
}
Mesh::Mesh(gltf::json &&object) noexcept
{
    for(auto field : object)
    {
        std::string_view const key = field.unescaped_key();
        simdjson::ondemand::value const value = field.value();

        FILL_FIELD(key, value, primitives   , valueToVector(valueTo<Primitive>));
        FILL_FIELD(key, value, name         , valueTo<std::string_view>);
    }
}

} // namespace gltf
