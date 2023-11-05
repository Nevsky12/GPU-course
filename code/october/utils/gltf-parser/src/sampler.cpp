#include <gltf/sampler.h>
#include "common.h"

namespace gltf
{

Sampler::Sampler(gltf::json &&object) noexcept
{
    for(auto field : object)
    {
        std::string_view const key = field.unescaped_key();
        simdjson::ondemand::value value = field.value();

        FILL_FIELD(key, value, magFilter, valueToEnum<MagFilter>);
        FILL_FIELD(key, value, minFilter, valueToEnum<MinFilter>);
        FILL_FIELD(key, value, wrapS    , valueToEnum<Wrap>);
        FILL_FIELD(key, value, wrapT    , valueToEnum<Wrap>);
        FILL_FIELD(key, value, name     , valueTo<std::string_view>);
    }
}

} // namespace gltf
