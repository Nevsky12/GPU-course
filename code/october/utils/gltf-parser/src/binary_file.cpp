#include <gltf/binary_file.h>
#include "common.h"

namespace gltf
{

void BinaryFile::Chunk::reserve()
{
    data.reserve(type == Chunk::JSON ? length + simdjson::SIMDJSON_PADDING : length);
    data.resize(length);
}
std::vector<u8> const &BinaryFile::findChunk(u32 const type) const noexcept
{
    return std::ranges::find(chunk, type, &Chunk::type)->data;
}

} // namespace gltf
