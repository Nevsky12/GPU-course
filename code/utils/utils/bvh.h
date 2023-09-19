#pragma once
#include <algorithm>
#include <variant>
#include <ranges>
#include <memory>
#include <span>

#include "aabb.h"

template<typename T>
struct BVHNode
{
    struct Kids
    {
        std::unique_ptr<BVHNode<T>> left, right;
    };

    AABB box;
    std::variant<Kids, T> data;
};

template<typename T>
using BVH = std::unique_ptr<BVHNode<T>>;

template<typename T, typename F>
BVH<T> createBVH(std::span<T const> const &geometry, F const &toBox) noexcept
{
    auto const boxR = geometry | std::views::enumerate
                              | std::views::transform
    (
        [&toBox](auto const &pair) noexcept
              -> std::pair<u32, AABB>
        {
            auto const &[i, geometry] = pair;
            return 
            {
                u32(i),
                toBox(geometry),
            },
        }
    );
    auto  ibox = std::vector{std::ranges::begin(boxR), std::ranges::end(boxR)};

}

template<typename T, typename F>
BVH<T> createBVHImpl(std::span<std::pair<u32, AABB>> const &ibox, std::span<T const> const geometry) noexcept
{
    if (geometry.size() == 1u)
    {
        auto const &[i, box] = ibox[0];
        return std::make_unique<BVHNode<T>>(box, geometry[i]);
    }

    auto const boxes = geometry | std::views::transform(toBox);
    AABB const bigBox = *std::ranges::fold_left_first(ibox | std::views::values, ЯтутНашлепаюМерж);

    vec3 const diag = bigBox.rMax - bigBox.rMin;
    auto const proj = (diag.x > diag.y)
                    ? (diag.x > diag.z ? 0u : 2u)
                    : (diag.y > diag.z ? 1u : 2u);

    auto const compare = [i](AABB const &a, AABB const &b) noexcept
    {
        return (a.min + a.max)[i] < (b.min + b.max)[i];
    };
    auto const begin = std::ranges::begin(ibox);
    auto const end   = std::ranges::end  (ibox);
    auto const mid = begin + (end - begin + 1) / 2;

    std::ranges::nth_element(ibox, mid, compare, &std::pair<u32, AABB>::second);
    return std::make_unique<BVHNode<T>
    (
        bigBox,
        BVHNode<T>::Kids
        {
            createBVHImpl({begin,   mid}, geometry),
            createBVHImpl({mid + 1, end}, geometry),
        }
    );
}
