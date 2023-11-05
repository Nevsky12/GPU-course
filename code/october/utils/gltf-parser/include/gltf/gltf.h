#pragma once
#include <gltf/binary_file.h>
#include <gltf/json.h>
#include <span>

namespace gltf
{

struct GLTF
{
    gltf::BinaryFile binary;
    gltf::JSON json;
    std::span<u8 const> bytes;

    template<typename IStream>
    GLTF(IStream &&in) noexcept
        : binary(static_cast<IStream &&>(in))
        , json  (binary.findChunk(gltf::BinaryFile::Chunk::JSON))
        , bytes (binary.findChunk(gltf::BinaryFile::Chunk:: BIN))
    {}
};

} // namespace gltf
