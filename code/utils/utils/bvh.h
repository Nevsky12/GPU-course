#pragma once
#include <algorithm>
#include <cassert>
#include <queue>
#include <stack>
#include <ranges>
#include <vector>
#include "aabb.h"

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
    using T = std::ranges::range_value_t<Boxable>;

    auto const boxR = r | std::views::transform(toBox);
    u32 const leafCount = u32(std::ranges::size(r));
    assert(leafCount != 0u);

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

        std::ranges::nth_element(ord, ord.begin() + mid, {}, proj);

        range.push({b, b + mid});
        range.push({b + mid, e});
    }

    u32 const fullLeft = 1u << u32(std::log2(leafCount - 1u)); 
    std::ranges::rotate(order, order.begin() + 2u * std::ptrdiff_t(leafCount - fullLeft));

    return BVH<T>
    {
        .boxes = boxes,
        .order = order,
        .geometry = std::move(r),
    };
}

template<typename T, typename I>
f32 hitDistance(std::pair<T const, I> const &intersection) 
{
    return hitDistance(intersection.second);
}

#ifdef STACKFULL

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
    using R = decltype(rayIntersection(ray, bvh, range));

    auto const &boxes = bvh.boxes;
    u32 const leafCount = bvh.leafCount();

    std::stack<u32> trail;
    trail.push(0u);

    R res = std::nullopt;
    f32 tMax = range.tMax;

    while(!trail.empty())
    {
        u32 const i = trail.top();
        trail.pop();
        
        if(i >= leafCount - 1u)
        {
            T const geomLeaf = bvh.leaf(i);
            auto const hit = rayIntersection(ray, geomLeaf, {range.tMin, tMax});
            if(hit)
            {
                f32 const t = hitDistance(*hit);
                if(t < tMax)
                {
                    tMax = t;
                    res = {geomLeaf, *hit};
                }
            }
        }
        else
        {
            u32 const goLeft  = 2u * i + 1u;
            u32 const goRight = 2u * i + 2u;
            auto const iLeft  = rayIntersection(ray, boxes[goLeft ], range);
            auto const iRight = rayIntersection(ray, boxes[goRight], range);

            if(iLeft  && hitDistance(*iLeft)  < tMax)
                trail.push(2u * i + 1u);
            if(iRight && hitDistance(*iRight) < tMax)
                trail.push(2u * i + 2u);              
        }
    }
    return res;
}

#else

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

TraverseState proceed(TraverseState const state, bool const goLeft, bool const goRight) noexcept
{
    if (state.node < state.count)
    {
        if(goLeft || goRight)
        {
            u32 const next = goLeft
                      ? 2u * state.node
                      : 2u * state.node + 1u;
            return 
            {
                .count = state.count,
                .node  = next,
                .trail = 2u * state.trail + (goLeft xor goRight ? 1u : 0u),
            };
        }
    }
    return up(state);
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
    using R = decltype(rayIntersection(ray, bvh, range));

    auto const &boxes = bvh.boxes;
    u32 const leafCount = bvh.leafCount();

    TraverseState state = initializeTraverse(leafCount);
    
    R res = std::nullopt;
    f32 tMax = range.tMax;

    while(incomplete(state))
    {   
        u32 const i = state.node - 1u;
        if(i >= leafCount + 1u)
        {
            T const geomLeaf = bvh.leaf(i);
            auto const hit = rayIntersection(ray, geomLeaf, {range.tMin, tMax});
            if(hit)
            {
                f32 const t = hitDistance(*hit);
                if(t < tMax)
                {
                    tMax = t;
                    res = {geomLeaf, *hit};
                }
            }
            state = up(state);
        }
        else
        {
            u32 const left  = 2u * i + 1u;
            u32 const right = 2u * i + 2u;
            auto const iLeft  = rayIntersection(ray, boxes[left ], range);
            auto const iRight = rayIntersection(ray, boxes[right], range);

            bool const goLeft   = iLeft && hitDistance(*iLeft) < tMax;
            bool const goRight = iRight && hitDistance(*iRight) < tMax;
            state = proceed(state, goLeft, goRight);
        }
    }
    return res;
}

#endif
