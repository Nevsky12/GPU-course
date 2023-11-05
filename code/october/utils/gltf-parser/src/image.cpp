#include <gltf/image.h>
#include "common.h"

namespace gltf
{

Image::Image(gltf::json &&object) noexcept
{
    for(auto field : object)
    {
        std::string_view const key = field.unescaped_key();
        simdjson::ondemand::value const value = field.value();

        FILL_FIELD(key, value, mimeType     , valueTo<std::string_view>);
        FILL_FIELD(key, value, bufferView   , valueToU32);
        FILL_FIELD(key, value, name         , valueTo<std::string_view>);
    }
}

} // namespace gltf
