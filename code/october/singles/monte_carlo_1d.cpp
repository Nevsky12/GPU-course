#include "random.h"
#include <iostream>
int main()
{
    auto const f = [](f32 const x) noexcept {return std::sqrt(4.f * x);};
    auto const p_f = [](f32 const y) noexcept {return y / 2.f;};

    auto const h = [](f32 const y) noexcept {return y * y * y;};

    auto const g = [&](f32 const y) noexcept {return h(y) / p_f(y);};
    auto const sampleG = [&]() noexcept
    {
        f32 const x = generate01N<1>()[0];
        return g(f(x));
    };

    auto const next = [](u32 const i) noexcept {return i + std::ceil((i / 100u) + 1u);};
    for(u32 n = 5u; n < 100000u; n = next(n))
    {
        f64 sumG  = 0.;
        f64 sumG2 = 0.;
        for(u32 i = 0u; i < n; ++i)
        {
            f32 const z = sampleG();
            sumG  += z;
            sumG2 += z * z;
        }
        f64 const E = sumG / f64(n);
        f64 const V = (sumG2 / f64(n) - E * E) / f32(n - 1);
        std::cout << n << ' ' << E << ' ' << std::sqrt(V) << std::endl;
    }
}
