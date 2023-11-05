#pragma once
#include <cassert>
#include <cmath>

#include <algorithm>
#include <numeric>
#include <ranges>
#include <vector>

struct Segment
{
    float min, max;

    bool contains(float const x) const noexcept {return min <= x && x <= max;}
    float center() const noexcept {return std::midpoint(min, max);}
    float length() const noexcept {return max - min;}
};
struct DiscretizedSegment
{
    Segment segment;
    unsigned int subsegmentCount;

    float boundary(unsigned int const i) const noexcept
    {
        return std::lerp(segment.min, segment.max, float(i) / float(subsegmentCount));
    }
    Segment subsegment(unsigned int const i) const noexcept
    {
        return
        {
            boundary(i     ),
            boundary(i + 1u),
        };
    }
    auto subsegmentRange() const noexcept
    {
        return std::views::iota(0u, subsegmentCount) | std::views::transform
        (
            [this](unsigned int const i) noexcept {return subsegment(i);}
        );
    }

    unsigned int find(float const x) const noexcept
    {
        assert(segment.contains(x));
        return std::min
        (
            subsegmentCount - 1u,
            unsigned(float(subsegmentCount) * (x - segment.min) / segment.length())
        );
    }
    template<typename F>
    auto valueRange(F &&func) const noexcept
    {
        return std::views::iota(0u, subsegmentCount) | std::views::transform
        (
            [this, f = static_cast<F &&>(func)](unsigned int const i) noexcept
            {
                return f(subsegment(i).center());
            }
        );
    }
};

struct Histogram
{
    DiscretizedSegment range;
    float weight;
    std::vector<unsigned int> bucket;

    template<std::ranges::range R>
    Histogram(DiscretizedSegment rangeS, R &&valueRange) noexcept
        : range(rangeS)
        , bucket(range.subsegmentCount, 0u)
    {
        unsigned int sampleCount = 0u;
        for(float const f : valueRange)
        {
            bucket[range.find(f)] += 1u;
            ++sampleCount;
        }
        weight = float(range.subsegmentCount) / (range.segment.length() * float(sampleCount));
    }

    struct Point
    {
        float   f;
        float pdf;
    };
    Point point(unsigned int const i) const noexcept
    {
        return
        {
            .f = range.subsegment(i).center(),
            .pdf = float(bucket[i]) * weight,
        };
    }
    auto pointRange() const noexcept
    {
        return std::views::iota(0u, range.subsegmentCount) | std::views::transform
        (
            [this](unsigned int const i) noexcept {return point(i);}
        );
    }

    Point operator()(float const f) const noexcept {return point(range.find(f));}
};
