#include <gltf/node.h>
#include "common.h"

namespace gltf
{

Node::Node(gltf::json &&object) noexcept
{
    for(auto field : object)
    {
        std::string_view const key = field.unescaped_key();
        simdjson::ondemand::value value = field.value();

        FILL_FIELD(key, value, mesh         , valueToU32);
        FILL_FIELD(key, value, children     , valueToVector(valueToU32));
        FILL_FIELD(key, value, name         , valueTo<std::string_view>);
        FILL_FIELD(key, value, matrix       , valueToMat4);
        FILL_FIELD(key, value, rotation     , valueToVec4);
        FILL_FIELD(key, value, scale        , valueToVec3);
        FILL_FIELD(key, value, translation  , valueToVec3);
    }
}

mat4 Node::transform() const noexcept
{
    if(any(matrix != identity<f32, 4>))
        return matrix;

    mat3 const R = gltf::fromQuaternion(rotation);
    mat3 const S = gltf::fromDiagonal(scale);
    mat3 const m = gltf::compose(R, S);
    return
    {
        {m[0], 0.f},
        {m[1], 0.f},
        {m[2], 0.f},
        {translation, 1.f},
    };
}

} // namespace gltf
