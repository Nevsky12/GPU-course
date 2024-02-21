#include <experimental/simd>
#include "types.h"
#include "gvec.h"

namespace stdx = std::experimental;
using ABI = stdx::simd_abi::fixed_size<4>;

using u32x4 = stdx::simd<u32, ABI>;

using f32x4 = stdx::simd<f32, ABI>;

using vec2   = utils::gvec<f32  , 2>;
using vec2x4 = utils::gvec<f32x4, 2>;

using vec3   = utils::gvec<f32  , 3>;
using vec3x4 = utils::gvec<f32x4, 3>;
using vec4x4 = utils::gvec<f32x4, 4>;

using f32Boolx4 = std::experimental::simd_mask<f32, ABI>;

