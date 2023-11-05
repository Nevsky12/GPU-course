#include <gltf/buffer.h>
#include "common.h"

namespace gltf
{

Buffer::Buffer(gltf::json &&object) noexcept
{
    for(auto field : object)
    {
        std::string_view const key = field.unescaped_key();
        simdjson::ondemand::value value = field.value();

        FILL_FIELD(key, value, byteLength   , valueToU32);
        FILL_FIELD(key, value, name         , valueTo<std::string_view>);
    }
}

} // namespace gltf
