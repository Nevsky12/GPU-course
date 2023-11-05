#pragma once
#include "binary_heap_bvh.h"
#include <gltf/view/geometry.h>
#include <gltf/utils/aabb.h>

#include <algorithm>
#include <stack>
#include <numeric>

namespace gltf::as
{

struct TopLevel
{
    BinaryHeapBVH<gltf::utils::AABB, u32> bvh;

    struct BLASInfo
    {
        u32 meshI, primitiveI;
        std::optional<u32> materialI;

        gltf::utils::AABB box;
        u32 begin, end;
    };
    std::vector<BLASInfo> blasInfo;

    struct Info
    {
        mat4x3 transform;
        struct Pack
        {
            u64 geometryI : 24;
            u64     nodeI : 24;
            u64 materialI : 15;
            u64    opaque : 1;
        } pack;
        static_assert(sizeof(Pack) == 8u);
    };
    std::vector<Info> tlasInfo;

    TopLevel() = default;
    TopLevel(gltf::GLTF const &gltf) noexcept
    {
        u32 triangleCount = 0u;
        for(u32 meshI = 0u; meshI < u32(gltf.json.meshes.size()); ++meshI)
        {
            gltf::Mesh const &mesh = gltf.json.meshes[meshI];
            for(u32 primitiveI = 0u; primitiveI < u32(mesh.primitives.size()); ++primitiveI)
            {
                gltf::Mesh::Primitive const &primitive = mesh.primitives[primitiveI];
                auto const [localBox, geom] = gltf::view::geometry(gltf)(primitive);
                u32 const count = u32(std::ranges::size(geom));
                blasInfo.push_back
                ({
                    .meshI      = meshI,
                    .primitiveI = primitiveI,
                    .materialI  = primitive.material,
                    .box        = localBox,
                    .begin      = triangleCount,
                    .end        = triangleCount + count,
                });
                triangleCount += count;
            }
        }

        u32 const nodeCount = u32(gltf.json.nodes.size());
        std::vector<mat4> nodeTransform(nodeCount, gltf::identity<f32, 4>);
        for(u32 const root : gltf.json.scenes[gltf.json.scene].nodes)
        {
            struct Entry
            {
                u32 node;
                mat4 transform;
            };
            std::queue<Entry> queue;
            queue.push({root, gltf::identity<f32, 4>});

            while(!queue.empty())
            {
                auto const [n, m] = queue.front();
                queue.pop();

                gltf::Node const &node = gltf.json.nodes[n];
                nodeTransform[n] = m * node.transform();
                for(u32 const c : node.children)
                    queue.push({c, nodeTransform[n]});
            }
        }

        auto const transformBox = [](mat4x3 const &m, gltf::utils::AABB const &aabb) noexcept
        {
            auto const &[a, b] = aabb;
            vec3 const v[] =
            {
                m * vec4{a.x, a.y, a.z, 1.f},
                m * vec4{a.x, a.y, b.z, 1.f},
                m * vec4{a.x, b.y, a.z, 1.f},
                m * vec4{a.x, b.y, b.z, 1.f},
                m * vec4{b.x, a.y, a.z, 1.f},
                m * vec4{b.x, a.y, b.z, 1.f},
                m * vec4{b.x, b.y, a.z, 1.f},
                m * vec4{b.x, b.y, b.z, 1.f},
            };
            return gltf::utils::AABB
            {
                std::accumulate(v + 1, v + 8, v[0], gltf::min<f32, 3>),
                std::accumulate(v + 1, v + 8, v[0], gltf::max<f32, 3>),
            };
        };

        std::vector<gltf::utils::AABB> box;
        for(u32 n = 0u; n < nodeCount; ++n)
        {
            gltf::Node const &node = gltf.json.nodes[n];
            if(node.mesh)
            {
                u32 const m = *node.mesh;
                mat4 const T = nodeTransform[n];
                mat4 const I = inverse(T);

                auto const begin = std::ranges::lower_bound(blasInfo, m, {}, &BLASInfo::meshI);
                auto const   end = std::ranges::upper_bound(blasInfo, m, {}, &BLASInfo::meshI);
                for(auto it = begin; it != end; ++it)
                {
                    auto const material = it->materialI;
                    tlasInfo.push_back
                    ({
                        .transform = mat4x3(I),
                        .pack =
                        {
                            .geometryI = 0xffffffull & u64(it - blasInfo.begin()),
                            .nodeI     = 0xffffffull & n,
                            .materialI = 0x007fffull & (material
                                ? u64(*material)
                                : gltf.json.materials.size()),
                            .opaque = material
                                ? gltf.json.materials[*material].alphaMode == gltf::Material::AlphaMode::Opaque
                                : true,
                        },
                    });
                    box.push_back(transformBox(mat4x3(T), it->box));
                }
            }
        }
        bvh = {box};
        tlasInfo = bvh.reordered(tlasInfo);
    }
};

} // namespace gltf::as

