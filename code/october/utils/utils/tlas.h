#pragma once
#include "types.h"

#include <gltf/as/top_level.h>
#include <gltf/as/bottom_level.h>

#include <gltf/utils/offset_point.h>
#include <gltf/utils/triangle.h>

using namespace gltf::utils;
struct RayBLASIntersection
{
    u32 triangleI;
    gltf::utils::RayTriangleIntersection rti;
};
struct RayTLASIntersection
{
    u32 instanceI;
    u32 geometryI;
    u32 triangleI;
    gltf::utils::RayTriangleIntersection rti;
};

inline auto intersectBLAS(auto const &blas) noexcept
{
    return [&blas](Ray const &ray, RayDistanceRange rdr) noexcept
        -> std::optional<RayBLASIntersection>
    {
        auto const &[bvh, triangle] = blas;
        std::optional<RayBLASIntersection> result;

        auto traverse = bvh.traverse();
        while(traverse.isIncomplete())
        {
            if(traverse.onLeaf())
            {
                u32 const i = traverse.leaf();
                RayTriangleIntersection const intersection = rayTriangleIntersection(ray, triangle[i].pos, rdr);
                if(happened(intersection))
                {
                    rdr.tFar = intersection.t;
                    result = {i, intersection};
                }
                traverse = traverse.next();
            }
            else
            {
                auto const iLeft  = rayAABBIntersection(ray, bvh.box[traverse. left()], rdr);
                auto const iRight = rayAABBIntersection(ray, bvh.box[traverse.right()], rdr);
                auto const down = traverse.down(nonempty(iLeft), nonempty(iRight), iLeft.tNear <= iRight.tNear);
                traverse = down
                    ? *down
                    : traverse.next();
            }
        }
        return result;
    };
}
inline auto intersectTLAS(auto const &tlas, auto const &blas) noexcept
{
    return [&](Ray const &ray, RayDistanceRange rdr = {0.f, 1.f / 0.f}) noexcept
        -> std::optional<RayTLASIntersection>
    {
        auto const &[bvh, blasInfo, tlasInfo] = tlas;
        std::optional<RayTLASIntersection> result;

        auto traverse = bvh.traverse();
        while(traverse.isIncomplete())
        {
            if(traverse.onLeaf())
            {
                u32 const i = traverse.leaf();
                //if(tlasInfo[i].pack.opaque)
                {
                    u32 const geometryI = tlasInfo[i].pack.geometryI;
                    Ray const blasRay =
                    {
                        tlasInfo[i].transform * vec4(ray.pos, 1.f),
                        tlasInfo[i].transform * vec4(ray.dir, 0.f),
                    };
                    auto const intersection = intersectBLAS(blas[geometryI])(blasRay, rdr);
                    if(intersection)
                    {
                        rdr.tFar = intersection->rti.t;
                        result = {i, geometryI, intersection->triangleI, intersection->rti};
                    }
                }
                traverse = traverse.next();
            }
            else
            {
                auto const iLeft  = rayAABBIntersection(ray, bvh.box[traverse. left()], rdr);
                auto const iRight = rayAABBIntersection(ray, bvh.box[traverse.right()], rdr);
                auto const down = traverse.down(nonempty(iLeft), nonempty(iRight), iLeft.tNear <= iRight.tNear);
                traverse = down
                    ? *down
                    : traverse.next();
            }
        }
        return result;
    };
};
