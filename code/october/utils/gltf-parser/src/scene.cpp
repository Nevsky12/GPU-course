#include <gltf/scene.h>
#include "common.h"

namespace gltf
{

Scene::Scene(gltf::json &&object) noexcept
{
    for(auto field : object)
    {
        std::string_view const key = field.unescaped_key();
        simdjson::ondemand::value value = field.value();

        FILL_FIELD(key, value, nodes, valueToVector(valueToU32));
        FILL_FIELD(key, value, name , valueTo<std::string_view>);
    }
}

} // namespace gltf
