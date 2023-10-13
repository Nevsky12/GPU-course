#pragma once
#include <algorithm>
#include <cassert>
#include <queue>
#include <memory>
#include <ranges>
#include <variant>
#include <vector>
#include "aabb.h"
#include <iostream>
#include <stack>

template<typename T>
struct BVH
{
    std::vector<AABB> boxes;
    std::vector<u32>  order;
    std::vector<T>    geometry;

    u32 leafCount() const noexcept {return u32(geometry.size());}
};

template<typename T>
T leaf(BVH<T> const &bvh, u32 const vertex) noexcept
{
    //std::cout << "vertexI - n + 1: " << vertex << " - " << bvh.leafCount() - 1u << std::endl;
    u32 const leafI = vertex - (bvh.leafCount() - 1u);
    std::cout << "leafI: " << leafI << std::endl;
    std::cout << "order leaf: " << bvh.order[leafI] << std::endl;
    return bvh.geometry[bvh.order[leafI]];
}

template<typename T>
bool isLeaf(BVH<T> const &bvh, u32 const node) noexcept
{
//    std::cout << "node: " << node << std::endl;
    return (node >= bvh.leafCount()     - 1u)
        && (node <  bvh.leafCount() * 2 - 1u); 
}

using IBox = std::pair<u32, AABB>;
template<std::ranges::random_access_range Boxable>
auto createBVHImpl(std::span<IBox> const ibox, Boxable &&r)
{
    std::cout << ibox.size() << std::endl;
    using T = std::ranges::range_value_t<Boxable>;

    assert(ibox.size() != 0);

    std::queue<std::pair<u32, u32>> range;
    std::vector<AABB> boxes;

    u32 const leafCount = u32(std::ranges::distance(ibox));
    range.push({0u, leafCount});

    std::vector<u32> order(leafCount);
    for(u32 k = 0u; k < leafCount; ++k)
        order[k] = k; 

    for (u32 i = 0u; i + 1u < leafCount; ++i)
    {
        auto const [b, e] = range.front();
        range.pop();

        u32 const n = e - b;
        AABB const lastBigBox = *std::ranges::fold_left_first(ibox | std::views::values
                                                                   | std::views::drop(b)
                                                                   | std::views::take(n), mergeBox);
        boxes.push_back(lastBigBox);

        vec3 const diag = lastBigBox.max - lastBigBox.min;
        auto const proj = 
        [
            i = diag.x > diag.y
                ? (diag.x > diag.z ? 0u : 2u)
                : (diag.y > diag.z ? 1u : 2u)
        ](IBox const &box) noexcept
        {
            auto const &[A, B] = box.second;
            return A[i] + B[i];
        };

        u32 const left = 1u << u32(std::log2(n - 1u));
        u32 const right = left >> 1u;
        u32 const mid = n < left + right
                      ? n -        right
                      :     left;

        auto const ib = ibox | std::views::drop(b)
                             | std::views::take(n);
        auto const begin = std::ranges::begin(ib);
        auto const end   = std::ranges::end  (ib);
        std::ranges::nth_element(begin, begin + mid, end, {}, proj);

       
        auto const ord = order | std::views::drop(b) 
                               | std::views::take(n);
        auto const isomer = [&](u32 const j) noexcept  
        {
            return proj(ib[j]);
        };
        
        std::ranges::nth_element(ord, std::ranges::begin(ord) + mid, {}, isomer);
   
        range.push({b, b + mid});
        range.push({b + mid, e});
    }

    auto const leafBoxes = ibox | std::views::values;
    boxes.insert(boxes.end(), leafBoxes.begin(), leafBoxes.end());

    u32 const fullLeft = 1u << u32(std::log2(leafCount - 1u));
    std::ranges::rotate(order, std::ranges::begin(order) + 2 * std::ptrdiff_t(leafCount - fullLeft));

    return BVH<T>
    {
        .boxes = boxes,
        .order = order,
        .geometry = std::move(r),
    };
}

template<std::ranges::random_access_range Boxable, typename F>
auto createBVH(Boxable &&r, F const &toBox)
    requires(requires(std::ranges::range_value_t<Boxable> const geom)
    {
        {toBox(geom)} -> std::convertible_to<AABB>;
    })
{
    auto const boxER = r | std::views::enumerate
                         | std::views::transform
        (
            [toBox](auto const &pair) noexcept
                -> IBox
            {
                auto const &[i, obj] = pair;
                return {u32(i), toBox(obj)};
            }
        );
    std::vector<IBox> box = {std::ranges::begin(boxER), std::ranges::end(boxER)};
    return createBVHImpl(box, std::forward<Boxable>(r));
}

template<typename A, typename B>
f32 hitDistance(std::pair<A const *, B> const &intersection) noexcept
{
    return hitDistance(intersection.second);
}

template<typename T>
auto rayIntersection(Ray const ray, BVH<T> const &bvh, RayRange const range) noexcept
    -> std::optional
    <
        std::pair
        <
            T,
            typename decltype(rayIntersection(ray, std::declval<T>(), range))::value_type
        >
    >
{
    auto const &boxes = bvh.boxes;

    std::stack<u32> trail;
    trail.push(1u);

    while(!trail.empty())
    {
        u32 const i = trail.top();
        trail.pop();

        if (!isLeaf(bvh, i - 1u))
        {
            auto const iLeft  = rayIntersection(ray, boxes[2u * i - 1u], range);
            auto const iRight = rayIntersection(ray, boxes[2u * i     ], range);
  
            if(iLeft && iRight)
            {
               f32 const tLeft  = hitDistance(*iLeft);
               f32 const tRight = hitDistance(*iRight);
               if(tLeft < tRight)
               {
                   trail.push(2u * i);
                   trail.push(2u * i + 1u);
               }
               else
               {
                   trail.push(2u * i + 1u);
                   trail.push(2u * i);
               }
            }
            else if(iLeft && !iRight)
                trail.push(2u * i);
            else if(!iLeft && iRight)
                trail.push(2u * i + 1u);
        }
        else
        {
            T const geometryLeaf = leaf(bvh, i - 1u);
            auto const [r0, r1, r2] = geometryLeaf;
            std::cout << "range: " << range.tMin << ", " << range.tMax << std::endl;
            std::cout << "x: " << r0.x << ", " << r1.x << ", " << r2.x << std::endl;
            auto const res = rayIntersection(ray, geometryLeaf, range).transform
            ( 
                [&]<typename I>(I const &intersection) noexcept
                {
                    return std::pair<T, I>
                    {
                        geometryLeaf,
                        intersection
                    };
                }
            );
            std::cout << "yes inter: " << res.has_value() << std::endl;
            return res;
        }
    }
    return std::nullopt;
}


/*
struct TraverseState
{
    u32 count;
    u32 node;
    u32 trail;
};

inline TraverseState initializeTraverse(u32 const count) noexcept
{
    return {count, 1u, 0u};
}

inline bool incomplete(TraverseState const state) noexcept
{
    return state.node != 0u;
}

u32 sibling(u32 const node) noexcept
{
    return node == 0u
         ? 0u
         : node +  1u - ((node & 1) << 1u);
}

TraverseState up(TraverseState const state) noexcept
{
    u32 const trail = state.trail + 1u;
    u32 const up = u32(std::countr_zero(trail));
    return
    {
        .count = state.count,
        .node  = sibling(state.node >> up),
        .trail = trail >> up,
    };
}

TraverseState down(TraverseState const state, bool const goLeft) noexcept
{
    u32 const node = goLeft
              ? state.node * 2u
              : state.node * 2u + 1u;
    return
    {
        .count = state.count,
        .node  = node,
        .trail = state.trail * 2u,
    };
}

TraverseState proceed(TraverseState const state, bool const control) noexcept
{
    return state.node < state.count
         ? down(state, control)
         : up  (state);
}
*/
