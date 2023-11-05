#pragma once
#include <cassert>
#include <cstdint>

#include <algorithm>
#include <concepts>
#include <numeric>
#include <optional>
#include <queue>
#include <ranges>
#include <vector>

#include <gltf/types.h>

namespace gltf::as
{

template<typename T>
concept AxisAlignedBoundingBox = requires()
{
    [](T const box) noexcept -> T                   // mergeable
    {
        auto const [a, b] = box;
        return {min(a, b), max(a, b)};
    };
    [](T const box, unsigned int const i) noexcept  // totally ordered along i-th axle
    {
        auto const [a, b] = box;
        return std::midpoint(a[i], b[i]);
    };
};

template<AxisAlignedBoundingBox AABB, std::unsigned_integral idx_t = unsigned int>
struct BinaryHeapBVH
{
    std::vector<AABB> box;
    std::vector<idx_t> order;

    /* this is a bounding volume hierarchy with binary heap topology;
     * a hierarchy is built from (n) boxes and contains (2 * n - 1) vertices
     *
     * enumerate the vertices from 0 to (2 * n - 2), then
     * definition of box: for every i < n - 1, box[i] = merge(box[2 * i + 1], box[2 * i + 2]);
     * i-th vertex is a parent, (2 * i + 1)-th vertex is a  left child
     *                        , (2 * i + 2)-th vertex is a right child;
     *
     * first (n - 1) vertices have both kids, next (n) vertices are leaves, so
     * if i is a vertex index, then l = i - (n - 1) is a leaf index
     *
     * the range (box | std::views::drop(n - 1)) contains boxes the hierarchy was constructed with but in a different order;
     * denote original index space as O, and leaf index space as L
     *
     * for any l in L, its corresponding index from O is order[l];
     * `reordered` method transforms a range index space from O to L: range[order[l]] == reordered(range)[l];
     *
     * for instance, if a hierarchy was created from boxR and std::ranges::size(boxR) == n,
     * then for every l < n, boxR[order[l]] == box[l + (n - 1)]
     *
     * summary:
     * 1.          initial box range: boxR      :: O -> AABB
     * 2.        hierarchy box range: box       :: V -> AABB
     * 3.       index transformation: order     :: L -> O
     * 4. range index transformation: reordered :: (O -> a) -> (L -> a)
     * 5. topology and index transform functions:
     *      children :: V -> (V, V)
     *      children i = (2 * i + 1, 2 * i + 2)
     *
     *      isLeaf :: V -> Bool
     *      isLeaf i = i >= n - 1
     *
     *      nodeToLeaf :: V -> L
     *      nodeToLeaf i = i - (n - 1)
     *
     *      leafToNode :: L -> V
     *      leafToNode l = l + (n - 1)
     */

    BinaryHeapBVH() = default;

    template<std::ranges::range BoxRange>
        requires(std::same_as<AABB, std::ranges::range_value_t<BoxRange>>)
    BinaryHeapBVH(BoxRange &&boxRange)
        : box  (std::ranges::size(boxRange) * 2u - 1u)
        , order(std::ranges::size(boxRange)          )
    {
        idx_t const n = idx_t(std::ranges::size(boxRange));
        assert(n != 0u);

        auto const boxR = box | std::views::drop(n - 1u);
        std::ranges::copy(boxRange, std::ranges::begin(boxR));

        for(idx_t i = 0u; i < n; ++i)
            order[i] = i;

        using Range = std::pair<idx_t, idx_t>;
        std::queue<Range> range;
        range.emplace(idx_t(0u), n);

        for(idx_t i = 0u; i + 1u < n; ++i)
        {
            auto const [b, e] = range.front();
            range.pop();

            auto const orderI = order | std::views::drop(b)
                                      | std::views::take(e - b);
            auto const tbox = orderI | std::views::transform([&boxR](idx_t const k) noexcept {return boxR[k];});
            auto const merge = [](AABB const &box1, AABB const &box2) noexcept
                -> AABB
            {
                auto const &[min1, max1] = box1;
                auto const &[min2, max2] = box2;
                return {min(min1, min2), max(max1, max2)};
            };

            box[i] = std::accumulate(tbox.begin() + 1, tbox.end(), tbox[0], merge);

            auto const &[imin, imax] = box[i];
            auto const diagonal = imax - imin;
            idx_t const j = diagonal.x > diagonal.y
                ? (diagonal.x > diagonal.z ? 0u : 2u)
                : (diagonal.y > diagonal.z ? 1u : 2u);

            auto const proj = [&boxR, j](idx_t const k) noexcept
            {
                auto const &[kmin, kmax] = boxR[k];
                return std::midpoint(kmin[j], kmax[j]);
            };

            idx_t const m = e - b;
            idx_t const p0 = 1u << idx_t(std::log2(m - 1u));
            idx_t const p1 = p0 >> 1u;
            idx_t const mid = m < p0 + p1 ? m - p1 : p0;

            std::ranges::nth_element(orderI, orderI.begin() + mid, {}, proj);

            range.emplace(b, b + mid);
            range.emplace(b + mid, e);
        }
        idx_t const p0 = 1u << idx_t(std::log2(n - 1u));
        std::ranges::rotate(order, order.begin() + 2 * std::ptrdiff_t(n - p0));

        reorder(boxR);
    }

    template<std::ranges::random_access_range R>
    R &&reorder(R &&range) const noexcept
    {
        assert(std::ranges::size(range) == order.size());
        for(idx_t const i : std::views::iota(idx_t(0u), idx_t(std::ranges::size(order))))
        {
            idx_t j = order[i];
            while(j < i)
                j = order[j];
            if(i != j)
                std::swap(range[i], range[j]);
        }
        return static_cast<R &&>(range);
    }
    template<std::ranges::range R>
    std::vector<std::ranges::range_value_t<R>> reordered(R &&range) const
    {
        auto r = std::views::common(static_cast<R &&>(range));
        return reorder(std::vector<std::ranges::range_value_t<R>>{std::ranges::begin(r), std::ranges::end(r)});
    }

    struct TraverseState
    {
        // https://jcgt.org/published/0002/01/03/

        idx_t count;
        idx_t node; // node is a shifted vertex index: node = i + 1u
                    // node = 0 is a completed traverse state
        idx_t trail;

        void    terminate()       noexcept {node = 0u;}

        bool   isComplete() const noexcept {return node == 0u;}
        bool isIncomplete() const noexcept {return node != 0u;}

        bool       onLeaf() const noexcept {return                          node >=  count ;}
        idx_t        leaf() const noexcept {assert( onLeaf()); return idx_t(node  -  count);} // index space L
        idx_t        left() const noexcept {assert(!onLeaf()); return idx_t(2u * node - 1u);} // index space V
        idx_t       right() const noexcept {assert(!onLeaf()); return idx_t(2u * node     );} // index space V

        TraverseState next() const noexcept
        {
            idx_t const  k = trail + 1u;
            idx_t const up = idx_t(std::countr_zero(k));
            idx_t const  n = node >> up;
            return TraverseState
            {
                count,
                n == idx_t(0u)
                    ? 0u
                    : n + 1u - ((n & 1u) << 1u),
                k >> up,
            };
        }
        std::optional<TraverseState> down( bool const goLeft
                                         , bool const goRight
                                         , bool const firstGoLeft
                                         ) const noexcept
        {
            return goLeft || goRight
                ? std::optional<TraverseState>
                {{
                    count,
                    2u * node + (!goRight || (goLeft && firstGoLeft) ? 0u : 1u),
                    2u * trail + (goRight xor goLeft ? 1u : 0u),
                }}
                : std::nullopt;
        }
    };
    TraverseState traverse() const noexcept {return {idx_t(order.size()), 1u, 0u};}
};

} // namespace gltf::as
