#include <iostream>
#include <cmath>

using u32 = unsigned int;
using f32 = float;

inline constexpr auto piecewiseGaussian(f32 const x0, f32 const a, f32 const b) noexcept
{
    return [=](f32 const x) noexcept
    {
        return std::exp(-0.5f * (x - x0) * (x - x0) * (x < x0 ? a * a : b * b));
    };
}

inline constexpr f32 X(f32 const l) noexcept
{
    return 1.056f * piecewiseGaussian(599.8f, 0.0264f, 0.0323f)(l)
         + 0.362f * piecewiseGaussian(442.0f, 0.0624f, 0.0374f)(l)
         - 0.065f * piecewiseGaussian(501.1f, 0.0490f, 0.0382f)(l);
}
inline constexpr f32 Y(f32 const l) noexcept
{
    return 0.821f * piecewiseGaussian(568.8f, 0.0213f, 0.0247f)(l)
         + 0.286f * piecewiseGaussian(530.9f, 0.0613f, 0.0322f)(l);
}
inline constexpr f32 Z(f32 const l) noexcept
{
    return 1.217f * piecewiseGaussian(437.0f, 0.0845f, 0.0278f)(l)
         + 0.681f * piecewiseGaussian(459.0f, 0.0385f, 0.0725f)(l);
}

int main()
{
    for(f32 l = 450.f; l < 650.f; l += 1.f)
    {
        f32 const x = X(l);
        f32 const y = Y(l);
        f32 const z = Z(l);
        f32 const m = x + y + z;
        std::cout << l << ' ' << x << ' ' << y << ' ' << z << ' ' << x / m << ' ' << y / m << std::endl;
    }
}
