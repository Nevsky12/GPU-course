#include <gltf/buffer_view.h>
#include "common.h"

namespace gltf
{

BufferView::BufferView(gltf::json &&object) noexcept
{
    for(auto field : object)
    {
        std::string_view const key = field.unescaped_key();
        simdjson::ondemand::value value = field.value();

        FILL_FIELD(key, value,     buffer, valueToU32);
        FILL_FIELD(key, value, byteOffset, valueToU32);
        FILL_FIELD(key, value, byteLength, valueToU32);
        FILL_FIELD(key, value, byteStride, valueToU32);
    }
}

} // namespace gltf
