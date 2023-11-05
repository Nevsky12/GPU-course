#pragma once
#include "buffer.h"
#include <numeric>

namespace gltf
{

namespace view
{

struct Triangle
{
    gvec<vec3, 3> pos;
    gvec<vec2, 3> tex;
    gvec<vec3, 3> norm;
};

inline auto geometry(gltf::GLTF const &file) noexcept
{
    return [&file](gltf::Mesh::Primitive const &primitive) noexcept
    {
        assert(primitive.attributes.POSITION   &&
               primitive.attributes.TEXCOORD_0 &&
               primitive.attributes.NORMAL);

        u32 const  posA = *primitive.attributes.POSITION;
        u32 const  texA = *primitive.attributes.TEXCOORD_0;
        u32 const normA = *primitive.attributes.NORMAL;

        auto const bufferView = gltf::view::buffer(file);
        auto const  pos = bufferView(file.json.accessors[ posA], bytesAsVec3);
        auto const  tex = bufferView(file.json.accessors[ texA], bytesAsVec2);
        auto const norm = bufferView(file.json.accessors[normA], bytesAsVec3);

        auto const &p0 = file.json.accessors[posA].min;
        auto const &p1 = file.json.accessors[posA].max;

        vec3 const pMin = p0.size() == 3
            ? vec3{p0[0], p0[1], p0[2]}
            : std::accumulate(std::next(pos.begin()), pos.end(), pos[0], gltf::min<f32, 3>);
        vec3 const pMax = p1.size() == 3
            ? vec3{p1[0], p1[1], p1[2]}
            : std::accumulate(std::next(pos.begin()), pos.end(), pos[0], gltf::max<f32, 3>);

        u32 const vertexCount =  u32(std::ranges::size( pos)) ;
        assert(   vertexCount == u32(std::ranges::size( tex)));
        assert(   vertexCount == u32(std::ranges::size(norm)));

        auto const indices = primitive.indices
            ? std::optional{bufferView(file.json.accessors[*primitive.indices], bytesAsIdx)}
            : std::nullopt;
        u32 const indicesCount = indices ? std::ranges::size(*indices) : vertexCount;
        auto const idx = std::views::iota(0u, indicesCount) | std::views::transform
        (
            [indices](u32 const i) noexcept
            {
                return indices ? (*indices)[i] : i;
            }
        );

        assert(primitive.mode == gltf::Mesh::Primitive::Mode::Triangles);
        return std::pair
        {
            gvec<vec3, 2>{pMin, pMax},
            std::views::iota(0u, indicesCount / 3u) | std::views::transform
            (
                [=](u32 const k) noexcept
                {
                    uvec3 const i = {idx[3 * k + 0], idx[3 * k + 1], idx[3 * k + 2]};
                    assert(i[0] < vertexCount);
                    assert(i[1] < vertexCount);
                    assert(i[2] < vertexCount);
                    return Triangle
                    {
                        .pos  = { pos[i[0]],  pos[i[1]],  pos[i[2]]},
                        .tex  = { tex[i[0]],  tex[i[1]],  tex[i[2]]},
                        .norm = {norm[i[0]], norm[i[1]], norm[i[2]]},
                    };
                }
            )
        };
    };
}

} // namespace view

} // namespace gltf
