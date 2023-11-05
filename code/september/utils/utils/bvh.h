#pragma once
#include <algorithm>
#include <cassert>
#include <memory>
#include <ranges>
#include <variant>
#include <vector>
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

using IBox = std::pair<u32, AABB>;
template<std::ranges::random_access_range Boxable>
auto createBVHImpl(std::span<IBox> const ibox, Boxable &&r)
{
    using T = std::ranges::range_value_t<Boxable>;

    assert(ibox.size() != 0);
    if(ibox.size() == 1)
    {
        auto const [i, box] = ibox[0];
        return std::make_unique<BVHNode<T>>(box, std::move(r[i]));
    }

    AABB const bigBox = *std::ranges::fold_left_first(ibox | std::views::values, mergeBox);

    vec3 const diag = bigBox.max - bigBox.min;
    auto const proj =
    [
        i = diag.x > diag.y
            ? (diag.x > diag.z ? 0u : 2u)
            : (diag.y > diag.z ? 1u : 2u)
    ](IBox const &box) noexcept
    {
        auto const &[a, b] = box.second;
        return a[i] + b[i];
    };

    auto const begin = std::ranges::begin(ibox);
    auto const   end = std::ranges::  end(ibox);
    auto const   mid = begin + (end - begin + 1u) / 2u;
    std::ranges::nth_element(begin, mid, end, {}, proj);
    return std::make_unique<BVHNode<T>>
    (
        bigBox,
        typename BVHNode<T>::Kids
        {
            .left  = createBVHImpl({begin, mid}, r),
            .right = createBVHImpl({  mid, end}, r),
        }
    );
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
            T const *,
            typename decltype(rayIntersection(ray, std::get<T>(bvh->data), range))::value_type
        >
    >
{
    if(std::holds_alternative<T>(bvh->data))
    {
        T const &geometry = std::get<T>(bvh->data);
        return rayIntersection(ray, geometry, range).transform
        (
            [&]<typename I>(I const &intersection) noexcept
            {
                return std::pair<T const *, I>
                {
                    &geometry,
                    intersection,
                };
            }
        );
    }
    auto const &[left, right] = std::get<typename BVHNode<T>::Kids>(bvh->data);
    auto const iLeft  = rayIntersection(ray,  left->box, range);
    auto const iRight = rayIntersection(ray, right->box, range);

    if(!iLeft && !iRight)
        return std::nullopt;
    if(!iLeft)
        return rayIntersection(ray, right, range);
    if(!iRight)
        return rayIntersection(ray,  left, range);

    f32 const tLeft  = hitDistance(*iLeft );
    f32 const tRight = hitDistance(*iRight);

    bool const lr = tLeft < tRight;
    auto const &closer  = lr ? left  : right;
    auto const &further = lr ? right :  left;

    auto const iC = rayIntersection(ray, closer, range);
    if(!iC)
        return rayIntersection(ray, further, range);

    f32 const tC = hitDistance(*iC);
    auto const iF = rayIntersection(ray, further, {range.tMin, tC});
    if(!iF)
        return iC;
    f32 const tF = hitDistance(*iF);
    return tC < tF ? iC : iF;
}
