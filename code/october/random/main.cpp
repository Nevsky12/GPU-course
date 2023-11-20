#include <utils/histogram.h>
#include <utils/squares.h>

u32 const sampleCount = 10'000'000u;
template<typename F>
    // variable : domain -> range
Histogram randomVariablePDF(Segment const domain, Segment const range, F const &variable) noexcept
{
    return
    {
        DiscretizedSegment{range, 200u},
        DiscretizedSegment{domain, sampleCount}.valueRange(variable)
    };
}
template<typename V>
Histogram randomVariablePDF(Segment const range, V &&randomVariable) noexcept
{
    return
    {
        DiscretizedSegment{range, 200u},
        std::views::iota(0u, sampleCount) | std::views::transform
        (
            [&](u32) noexcept {return randomVariable();}
        )
    };
}

template<typename V>
auto randomVariableRange(V &&var, u32 const n) noexcept
{
    return std::views::iota(0u, n) | std::views::transform
    (
        [f = static_cast<V &&>(var)](u32) noexcept {return f();}
    );
};

template<typename V>
auto sameVariableSum(V &&var, u32 const n) noexcept
{
    return [f = static_cast<V &&>(var), n]() noexcept
    {
        auto const rvR = randomVariableRange(f, n);
        return std::accumulate(rvR.begin(), rvR.end(), 0.f) / float(n);
    };
};
template<typename V1, typename V2>
auto randomVariableProduct(V1 &&v1, V2 &&v2) noexcept
{
    return [f1 = static_cast<V1 &&>(v1), f2 = static_cast<V2 &&>(v2)]() noexcept
    {
        return f1() * f2();
    };
}

template<u32 N>
constexpr std::array<f32, N> generate01N() noexcept // -> [0.f, 1.f)^N
{
    return []<u32... i>(std::integer_sequence<u32, i...>) noexcept
    {
        static thread_local u64 counter = 0;
        return std::array
        {
            uniformFloat(uniformU32(++counter + (i - i)))...
        };
    }(std::make_integer_sequence<u32, N>{});
}
template<u32 N, typename F, typename PF, typename PG>
auto rejectionSample(F const &f, PF const &pdfF, PG const &pdfG, f32 const c) noexcept
{
    for(;;)
    {
        auto const xs = generate01N<N>();
        f32  const x = generate01N<1>()[0];

        auto const y = f(xs);
        assert(pdfF(y) >= c * pdfG(y));
        if(x * pdfF(y) >= c * pdfG(y))
            continue;
        return y;
    }
}
#include <iostream>
int main()
{
    /*
    auto const prod = randomVariableProduct(generateUniformFloat, generateUniformFloat);
    Histogram const hist = randomVariablePDF({0.f, 1.f}, prod);
    */

    /*
    auto const var = []() noexcept {f32 const x = generateUniformFloat(); return std::acos(x);};
    Histogram const hist = randomVariablePDF({0.f, 0.5f * 3.1415926535f}, var);
    */

    Histogram const hist = randomVariablePDF
    (
        {0.f, 2.f},
        []() noexcept
        {
            return rejectionSample<1>
            (
                [](std::array<f32, 1> const x) noexcept {return std::sqrt(4.f * x[0]);},
                [](f32 const y) noexcept {return 0.5f * y;},
                [](f32 const y) noexcept {return 3.f / 8.f * y * y;},
                2.f / 3.f
            );
        }
    );

    /*
    Histogram const hist = randomVariablePDF
    (
        {0.f, 1.f},
        {0.f, 0.5f * 3.1415926535f},
        [](f32 const x) noexcept {return std::acos(x);}
    );
    */

    /*
    auto const var = sameVariableSum(generateUniformFloat, 10u);
    Histogram const hist = randomVariablePDF({0.f, 1.f}, var);
    */

    for(auto const [f, pdf] : hist.pointRange())
    {
        std::cout << f << ' ' << pdf << std::endl;
    }
}
