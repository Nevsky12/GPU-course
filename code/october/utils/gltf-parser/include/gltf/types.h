#pragma once
#include <cstdint>
#include <cstddef>

#include <optional>
#include <string_view>
#include <vector>

#include <gltf/gvec.h>

namespace gltf
{

struct json;

using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using f32 = float;
using f64 = double;

using  vec2 = gvec<f32, 2>;
using ivec2 = gvec<i32, 2>;
using uvec2 = gvec<u32, 2>;
using dvec2 = gvec<f64, 2>;
using bvec2 = gvec<bool, 2>;

using  vec3 = gvec<f32, 3>;
using ivec3 = gvec<i32, 3>;
using uvec3 = gvec<u32, 3>;
using dvec3 = gvec<f64, 3>;
using bvec3 = gvec<bool, 3>;

using  vec4 = gvec<f32, 4>;
using ivec4 = gvec<i32, 4>;
using uvec4 = gvec<u32, 4>;
using dvec4 = gvec<f64, 4>;
using bvec4 = gvec<bool, 4>;

using  mat2 = gmat<f32, 2, 2>;
using  mat3 = gmat<f32, 3, 3>;
using  mat4 = gmat<f32, 4, 4>;

using  mat2x3 = gmat<f32, 2, 3>;
using  mat2x4 = gmat<f32, 2, 4>;
using  mat3x2 = gmat<f32, 3, 2>;
using  mat4x2 = gmat<f32, 4, 2>;

using  mat3x4 = gmat<f32, 3, 4>;
using  mat4x3 = gmat<f32, 4, 3>;

};
