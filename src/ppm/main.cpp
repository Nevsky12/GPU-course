#include <fstream>
#include <cmath>
#include <cstdint>

using u8  = std::uint8_t;
using u32 = std::uint32_t;
using f32 = float;

inline u32 toU8(f32 const x) noexcept
{
    return std::round(255.f * x);
}
inline f32 encode(f32 const x) noexcept
{
    return x < 0.0031308f
        ? x * 12.92f
        : 1.055f * std::pow(x, 1.f / 2.4f) - 0.055f;
}

int main()
{
    std::ofstream perceptual("perceptual.ppm");
    std::ofstream   physical(  "physical.ppm");

    u32 const width = 1920u;
    u32 const height = 1080u;

    perceptual << "P3\n" << width << ' ' << height << "\n255\n";
      physical << "P3\n" << width << ' ' << height << "\n255\n";

    for(u32 y = 0u; y < height; ++y)
    for(u32 x = 0u; x <  width; ++x)
    {
        f32 const fx = f32(x) / f32( width);
        f32 const fy = f32(y) / f32(height);

        perceptual << toU8(fx) << ' ' << toU8(fy) << ' ' << 0u << ' ';
        physical << toU8(encode(fx)) << ' ' << toU8(encode(fy)) << ' ' << 0u << ' ';
    }
}
