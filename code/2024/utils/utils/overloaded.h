#pragma once
#include <ranges>
#include "sphere.h"

template<typename Class, typename Member>
auto operator|(std::vector<Class> const& r, Member Class::* field )
{     
    return r | std::views::transform([field](auto const &el){return el.*field;});
}

template <std::ranges::range R>
auto to_vector(R&& r) 
{
    auto const rCommon = r | std::views::common;
    return std::vector(rCommon.begin(), rCommon.end());
}
