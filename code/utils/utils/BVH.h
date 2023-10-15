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

using IBox = std::pair<u32, AABB>;
template<std::ranges::random_access_range Boxable>
auto createBVHImpl(std::span<IBox const> const ibox, Boxable &&r) noexcept
{
    assert(ibox.size() != 0u);
    using T = std::ranges::range_value_t<Boxable>;


    u32 const leafCount = u32(std::ranges::distance(ibox));
    std::vector<AABB> boxes;
    std::vector<u32 > order(leafCount);
    for(u32 k = 0u; k < leafCount; ++k)
        order[k] = ibox[k].first;


    std::queue<std::pair<u32, u32>> range;
    range.push({0u, leafCount});


    AABB const bigBox  = *std::ranges::fold_left_first(ibox | std::views::values, mergeBox);
    vec3 const bigDiag = bigBox.max - bigBox.min;
    auto const proj =
    [
        i =  bigDiag.x > bigDiag.y
          ? (bigDiag.x > bigDiag.z ? 0u : 2u)
          : (bigDiag.y > bigDiag.z ? 1u : 2u)
    ](IBox const &ib) noexcept
    {
        auto const [A,     B] = ib.second;
        return      A[i] + B[i];
    };


    auto const fromOrd = [&](u32 const j) noexcept {return ibox[j];};//*std::ranges::find(ibox, order[j], &IBox::first);};
    auto const isomer  = [&](u32 const j) noexcept
    {
       return proj(fromOrd(j));
    };

    for(u32 i = 0u; i + 1u < leafCount; ++i)
    {
        auto const [b, e] = range.front();
        range.pop();


        u32 const n = e - b;
        u32 const left  = 1u << u32(std::log2(n - 1u));
        u32 const right = left >> 1u;
        u32 const mid = n < left + right
                      ? n - right
                      : left;
        std::cout << "e, b, n, mid: " << e << ", " << b << ", " << n << ", " << mid << std::endl;


        auto const ord = order | std::views::drop(b)
                               | std::views::take(n);


        AABB const lastBigBox = *std::ranges::fold_left_first(ord | std::views::transform(fromOrd) 
                                                                  | std::views::values,  mergeBox); 
        boxes.push_back(lastBigBox);


        std::ranges::nth_element(ord, std::ranges::begin(ord) + mid, {}, isomer);


        range.push({b, b + mid});
        range.push({b + mid, e});
    }



    //auto const fromOrd = [&](u32 const j) noexcept {return ibox[j];};//*std::ranges::find(ibox, order[j], &IBox::first);};
    //auto const leafBoxes = ibox | std::views::values;
    auto const leafBoxes = order | std::views::transform(fromOrd) | std::views::values;
    boxes.insert(boxes.end(), leafBoxes.begin(), leafBoxes.end());

    /*   
    u32 const fullLeft = 1u << u32(std::log2(leafCount - 1u));
    std::ranges::rotate(order, std::ranges::begin(order) + 2u * std::ptrdiff_t(leafCount - fullLeft));
    */

    return BVH<T>
    {
        .boxes    = boxes,
        .order    = order,
        .geometry = std::move(r)
    };
}


template<std::ranges::random_access_range Boxable, typename F>
auto createBVH(Boxable &&r, F const &toBox) noexcept
                -> BVH<std::ranges::range_value_t<Boxable>>
     requires(requires(std::ranges::range_value_t<Boxable> &&geom) 
     {
         {toBox(geom)} -> std::convertible_to<AABB>;
     })
{
    auto const boxER = r | std::views::enumerate 
                         | std::views::transform
    (
        [&toBox](auto const &pair) noexcept
        -> IBox
        {
            auto const &[i, obj] = pair;
            return {u32(i), toBox(obj)};
        } 
    );

    std::vector<IBox> box = {std::ranges::begin(boxER), std::ranges::end(boxER)};
    return createBVHImpl(box, std::forward<Boxable>(r));
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
            if(iLeft && iRight)
            {
                auto const [tlMin, tlMax] = *iLeft;
                auto const [trMin, trMax] = *iRight;
                
                if(tlMin < trMin)
                {
                    trail.push(2u * i + 1u);
                    trail.push(2u * i + 2u);
                }
                else
                {
                    trail.push(2u * i + 2u);
                    trail.push(2u * i + 1u);
                } 
            }
            else if(!iLeft &&  iRight)
                trail.push(2u * i + 2u);
            else if( iLeft && !iRight)
                trail.push(2u * i + 1u);
        }
    }

    return std::nullopt;
}
