#pragma once
#include <gltf/types.h>
#include <istream>

namespace gltf
{

struct BinaryFile
{
    struct Header
    {
        u32 magic;
        u32 version;
        u32 length;

        template<typename IStream>
        Header(IStream &in)
        {
            in.read(reinterpret_cast<typename IStream::char_type *>(&  magic), 4);
            in.read(reinterpret_cast<typename IStream::char_type *>(&version), 4);
            in.read(reinterpret_cast<typename IStream::char_type *>(& length), 4);
        }
    };
    struct Chunk
    {
    private:
        void reserve(); // hides simdjson::SIMDJSON_PADDING usage

    public:
        u32 length;
        u32 type;
        std::vector<u8> data;

        static constexpr u32 JSON = 0x4e4f534a;
        static constexpr u32  BIN = 0x004e4942;

        template<typename IStream>
        Chunk(IStream &in)
        {
            in.read(reinterpret_cast<typename IStream::char_type *>(&length), 4);
            in.read(reinterpret_cast<typename IStream::char_type *>(&  type), 4);
            reserve();
            in.read(reinterpret_cast<typename IStream::char_type *>(data.data()), length);
        }
    };

    Header header;
    std::vector<Chunk> chunk;

    template<typename IStream>
    BinaryFile(IStream &&in)
        : header(in)
    {
        while(in && in.tellg() != header.length)
            chunk.emplace_back(in);
    }

    std::vector<u8> const &findChunk(u32 const type) const noexcept;
};

} // namespace gltf
