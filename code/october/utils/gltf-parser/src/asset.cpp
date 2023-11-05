#include <gltf/asset.h>
#include "common.h"

namespace gltf
{

Asset::Asset(gltf::json &&object) noexcept
{
    for(auto field : object)
    {
        std::string_view const key = field.unescaped_key();
        simdjson::ondemand::value value = field.value();

        FILL_FIELD(key, value, copyright    , valueTo<std::string_view>);
        FILL_FIELD(key, value, generator    , valueTo<std::string_view>);
        FILL_FIELD(key, value, version      , valueTo<std::string_view>);
        FILL_FIELD(key, value, minVersion   , valueTo<std::string_view>);
    }
}

} // namespace gltf
