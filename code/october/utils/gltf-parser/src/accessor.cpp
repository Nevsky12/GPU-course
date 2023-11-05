#include <gltf/accessor.h>
#include "common.h"

namespace gltf
{

Accessor::Accessor(gltf::json &&json) noexcept
{
    auto const valueToType = [](simdjson::ondemand::value value) noexcept
    {
        std::string_view const sv = {value};
        if(sv == "VEC2")
            return Type::Vec2;
        if(sv == "VEC3")
            return Type::Vec3;
        if(sv == "VEC4")
            return Type::Vec4;
        if(sv == "MAT3")
            return Type::Mat3;
        if(sv == "MAT4")
            return Type::Mat4;
        assert(sv == "SCALAR");
        return Type::Scalar;
    };

    for(auto field : json)
    {
        std::string_view const key = field.unescaped_key();
        simdjson::ondemand::value value = field.value();

        FILL_FIELD(key, value, bufferView   , valueToU32);
        FILL_FIELD(key, value, byteOffset   , valueToU32);
        FILL_FIELD(key, value, componentType, valueToEnum<ComponentType>);
        FILL_FIELD(key, value, normalized   , valueTo<bool>);
        FILL_FIELD(key, value, count        , valueToU32);
        FILL_FIELD(key, value, type         , valueToType);
        FILL_FIELD(key, value, max          , valueToVector(valueToF32));
        FILL_FIELD(key, value, min          , valueToVector(valueToF32));
        FILL_FIELD(key, value, name         , valueTo<std::string_view>);
    }
}

} // namespace gltf
