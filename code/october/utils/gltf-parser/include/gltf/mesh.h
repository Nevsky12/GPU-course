#pragma once
#include <gltf/types.h>

namespace gltf
{

struct Mesh
{
    struct Primitive
    {
        struct Attributes
        {
            std::optional<u32> POSITION, TEXCOORD_0, NORMAL, TANGENT;

            Attributes() noexcept = default;
            Attributes(gltf::json &&) noexcept;
        };
        Attributes attributes;
        std::optional<u32> indices, material;

        enum class Mode
        {
            Points          = 0,
            Lines           = 1,
            LineLoop        = 2,
            LineStrip       = 3,
            Triangles       = 4,
            TriangleStrip   = 5,
            TriangleFan     = 6,
        } mode = Mode::Triangles;

        Primitive(gltf::json &&) noexcept;
    };

    std::vector<Primitive> primitives;
    std::string_view name;

    Mesh(gltf::json &&) noexcept;
};

} // namespace gltf
