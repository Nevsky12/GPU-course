#include <experimental/simd>
#include "types.h"

namespace stdx = std::experimental;

template<std::size_t N>
using ABI = stdx::simd_abi::fixed_size<N>;

template<std::size_t N>
using vf32t = stdx::simd<f32, ABI<N>>;
