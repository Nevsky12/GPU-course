#pragma once
#include <gltf/types.h>
#include <include/simdjson.h>
#include <ranges>

#define FILL_FIELD(key, value, name, transform) \
    if(key == #name) name = transform(value)

namespace gltf
{

struct json : simdjson::ondemand::object
{
    json(simdjson::ondemand::value value)
        : simdjson::ondemand::object(value)
    {}
};

template<typename T, typename U = T>
T valueTo(simdjson::ondemand::value value) noexcept {return static_cast<T>(U(value));}

inline constexpr auto valueToU32 = valueTo<u32, u64>;
inline constexpr auto valueToF32 = valueTo<f32, f64>;

template<typename T>
inline constexpr auto valueToEnum = valueTo<T, u64>;

template<typename F>
constexpr auto valueToVector(F const &transform) noexcept
{
    return [transform](simdjson::ondemand::value value) noexcept
    {
        std::vector<decltype(transform(value))> v;
        for(simdjson::ondemand::value x : value.get_array())
            v.push_back(transform(x));
        return v;
    };
}
template<typename T, typename F>
constexpr auto valueToArray(F const &transform) noexcept
{
    return [transform](simdjson::ondemand::value value) noexcept
    {
        T v;
        u32 i = 0u;
        for(simdjson::ondemand::value x : value.get_array())
            v[i++] = transform(x);
        assert(i == std::ranges::size(v));
        return v;
    };
}
inline constexpr auto valueToVec3 = valueToArray<vec3>(valueToF32);
inline constexpr auto valueToVec4 = valueToArray<vec4>(valueToF32);
inline constexpr auto valueToMat3 = valueToArray<mat3>(valueToVec3);
inline constexpr auto valueToMat4 = valueToArray<mat4>(valueToVec4);

} // namespace gltf
