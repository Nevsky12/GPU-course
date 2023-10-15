#pragma once
#include <algorithm>
#include <cassert>
#include <queue>
#include <stack>
#include <ranges>
#include <vector>
#include "aabb.h"

#include <iostream>

template<typename T>
struct BVH
{
    std::vector<AABB> boxes;
    std::vector<u32 > order;
    std::vector<T   > geometry;

    u32 leafCount(           ) const noexcept {return u32(geometry.size());}
    T   leaf     (u32 const v) const noexcept
    {
        u32 const l = v - (leafCount() - 1u);
        return geometry[order[l]];
    }
};


template<std::ranges::random_access_range Boxable, typename F>
auto createBVH(Boxable &&r, F const &toBox) noexcept
                -> BVH<std::ranges::range_value_t<Boxable>>
     requires(requires(std::ranges::range_value_t<Boxable> &&geom) 
     {
         {toBox(geom)} -> std::convertible_to<AABB>;
     })
{
    assert(r.size() != 0u);
    using T = std::ranges::range_value_t<Boxable>;

    auto const boxR = r | std::views::transform(toBox);
    AABB const bigBox = *std::ranges::fold_left_first(boxR, mergeBox);
    vec3 const diag = bigBox.max - bigBox.min;


    auto const proj =
    [
        i =  diag.x > diag.y
          ? (diag.x > diag.z ? 0u : 2u)
          : (diag.y > diag.z ? 1u : 2u)
    ](AABB const &box) noexcept
    {
        auto const [A,     B] = box;
        return      A[i] + B[i];
    };
    auto const isomer  = [&](u32 const j) noexcept
    {
        return proj(boxR[j]);
    };
    auto const fromOrd = [&](u32 const j) noexcept
    {
        return boxR[j];
    };


    u32 const leafCount = u32(std::ranges::distance(r));
    std::vector<AABB> boxes;
    std::vector<u32 > order(leafCount);
    for(u32 k = 0u; k < leafCount; ++k)
        order[k] = k;


    std::queue<std::pair<u32, u32>> range;
    range.push({0u, leafCount});


    for(u32 i = 0u; i + 1u < 2u * leafCount - 1u; ++i)
    {
        auto const [b, e] = range.front();
        range.pop();
 

        u32 const n = e - b;
        u32 const left  = 1u << u32(std::log2(n - 1u));
        u32 const right = left >> 1u;
        u32 const mid = n < left + right
                      ? n - right
                      : left;

//        std::cout << "e, b, n, mid: " << e << ", " << b << ", " << n << ", " << mid << std::endl;

        AABB const lastBigBox = *std::ranges::fold_left_first(order | std::views::drop(b) 
                                                                    | std::views::take(n)
                                                                    | std::views::transform(fromOrd), mergeBox);
        boxes.push_back(lastBigBox);
        std::ranges::nth_element(order, std::ranges::begin(order) + mid, {}, isomer);


        range.push({b, b + mid});
        range.push({b + mid, e});
    }

    u32 const fullLeft = 1u << u32(std::log2(leafCount - 1u));
    std::ranges::rotate(order, std::ranges::begin(order) + 2u * std::ptrdiff_t(leafCount - fullLeft));

    return BVH<T>
    {
        .boxes = boxes,
        .order = order,
        .geometry = std::move(r)
    };
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
    u32 const leafCount = bvh.leafCount();

    std::stack<u32> trail;
    trail.push(0u);

    while(!trail.empty())
    {
        u32 const i = trail.top();
        trail.pop();

        
        if(i + 1u >= leafCount)
        {
            T const geomLeaf = bvh.leaf(i);
            return rayIntersection(ray, geomLeaf, range).transform
            (
                [&]<typename I>(I const &intersection) noexcept
                {
                    return std::pair<T, I>
                    {
                         geomLeaf,
                         intersection
                    };
                }
            );
        }
        else
        {
            auto const iLeft  = rayIntersection(ray, boxes[2u * i + 1u], range);
            auto const iRight = rayIntersection(ray, boxes[2u * i + 2u], range);
                      
            if( iLeft && !iRight)
                trail.push(2u * i + 1u);
            if(!iLeft &&  iRight)
                trail.push(2u * i + 2u);
            if( iLeft &&  iRight)
            {
                auto const [tlMin, tlMax] = *iLeft;
                auto const [trMin, trMax] = *iRight;
                if(tlMin < trMin)
                {
                    trail.push(2u * i + 2u);
                    trail.push(2u * i + 1u);
                }
                else
                {
                    trail.push(2u * i + 1u);
                    trail.push(2u * i + 2u);
                }
            }
        }
    }
    return std::nullopt;
}
