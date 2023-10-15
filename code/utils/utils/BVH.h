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
    u32 const leafCount = u32(std::ranges::distance(r));

    std::vector<AABB> boxes;
    std::vector<u32 > order(leafCount);
    for(u32 k = 0u; k < leafCount; ++k)
        order[k] = k;

    std::queue<std::pair<u32, u32>> range;
    range.push({0u, leafCount});

    for(u32 i = 0u; i < 2u * leafCount - 1u; ++i)
    {
        auto const [b, e] = range.front();
        range.pop(); 

        u32 const n = e - b;
        std::cout << "n: " << n << std::endl;

        u32 const left  = 1u << u32(std::log2(n - 1u));
        u32 const right = left >> 1u;
        u32 const mid = n < left + right
                      ? n        - right
                      :     left;

        auto const ord = order | std::views::drop(b) 
                               | std::views::take(n);
        AABB const lastBigBox = *std::ranges::fold_left_first
        (
            ord | std::views::transform([&](u32 const j) noexcept {return boxR[j];})
            , 
            mergeBox
        );
        boxes.push_back(lastBigBox);
        vec3 const diag = lastBigBox.max - lastBigBox.min;

        auto const proj =
        [
            i =  diag.x > diag.y
              ? (diag.x > diag.z ? 0u : 2u)
              : (diag.y > diag.z ? 1u : 2u)
            ,
            &boxR
        ](u32 const j) noexcept
        {
            auto const [A    , B  ] = boxR[j];
            return      A[i] + B[i];
        };

        std::ranges::nth_element(ord, std::ranges::begin(ord) + mid, {}, proj);

        range.push({b, b + mid});
        range.push({b + mid, e});
    }

    /*
    std::cout << boxes.size() << ", " << 2u * leafCount - 1u << std::endl;
    assert(boxes.size() == 2u * leafCount - 1u);
    */

    u32 const fullLeft = 1u << u32(std::log2(leafCount - 1u)); 
    std::ranges::rotate(order, std::ranges::begin(order) + 2u * std::ptrdiff_t(leafCount - fullLeft));
 
    return BVH<T>
    {
        .boxes = boxes,
        .order = order,
        .geometry = std::move(r),
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
            u32 const goLeft  = 2u * i + 1u;
            u32 const goRight = 2u * i + 2u;
            auto const iLeft  = rayIntersection(ray, boxes[goLeft ], range);
            auto const iRight = rayIntersection(ray, boxes[goRight], range);
                      
            if( iLeft && !iRight)
                trail.push(goLeft);
            if(!iLeft && iRight)
                trail.push(goRight);
            if( iLeft &&  iRight)
            {
                auto const [tlMin, tlMax] = *iLeft;
                auto const [trMin, trMax] = *iRight;
                if(tlMin < trMin)
                {
                    trail.push(goRight);
                    trail.push(goLeft );
                }
                else
                {
                    trail.push(goLeft );
                    trail.push(goRight);
                }
            }
        }
    }
    return std::nullopt;
}
