#pragma once
#include "binary_heap_bvh.h"
#include <gltf/view/geometry.h>
#include <gltf/utils/aabb.h>
namespace gltf::as
{

template<typename AABB, typename Triangle>
struct BottomLevel
{
    BinaryHeapBVH<AABB, u32> bvh;
    std::vector<Triangle> triangle;

    template<std::ranges::range TriangleR, typename BoxF, typename TriF>
    BottomLevel( TriangleR &&triangleR
               , BoxF const &     boxTransform = lambdaE1(x, x) // gltf::utils::AABB     -> AABB
               , TriF const &triangleTransform = lambdaE1(x, x) // gltf::view ::Triangle -> Triangle
               , gltf::mat4x3 const &transform = {gltf::identity<f32, 4>}
               ) noexcept
        requires(requires( std::ranges::range_value_t<TriangleR> const tri
                         , BoxF const toBox
                         , TriF const toTri
                         , gltf::utils::AABB const aabb
                         )
        {
            {       tri } -> std::convertible_to<gltf::view::Triangle>;
            {toTri( tri)} -> std::convertible_to<Triangle>;
            {toBox(aabb)} -> std::convertible_to<AABB>;
        })
    {
        auto triR = triangleR | std::views::transform
        (
            [transform](gltf::view::Triangle const &t) noexcept
                -> gltf::view::Triangle
            {
                auto const [pos, tex, norm] = t;
                return {liftA1(lambdaR1(r, transform * vec4(r, 1.f)), pos), tex, norm};
            }
        );

        auto const toBox = +[](gltf::view::Triangle const &t) noexcept
        {
            return gltf::utils::AABB
            {
                foldl1(min<f32, 3>, t.pos),
                foldl1(max<f32, 3>, t.pos),
            };
        };

        auto boxR = triR | std::views::transform(toBox)
                         | std::views::transform(boxTransform)
                         | std::views::common;
        std::vector<AABB> const box = {std::ranges::begin(boxR), std::ranges::end(boxR)};

        bvh = {box};
        triangle = bvh.reordered(triR | std::views::transform(triangleTransform));
    }
};

} // namespace gltf::as
