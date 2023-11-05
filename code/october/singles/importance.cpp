#include <iostream>
#include <random>

float func(float x)
{
    return x * x;
}

int main()
{
    std::mt19937_64 gen(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.f, 1.f);

    auto const sampleU = [&]() {return dist(gen);};

    auto const sampleS1 = [&]() {return std::pair{4.f * sampleU(), 0.25f};};
    auto const sampleS2 = [&]()
    {
        float const x = 4.f * std::sqrt(sampleU());
        return std::pair
        {
            x,
            x / 8.f,
        };
    };
    auto const sampleS3 = [&]()
    {
        float const x = 4.f * std::cbrt(sampleU());
        return std::pair
        {
            x,
            3.f / 64.f * x * x,
        };
    };

    float accum[3] = {0.f, 0.f, 0.f};
    auto const N = 1000u;
    for(auto i = 0u; i < N; ++i)
    {
        {
            auto const [x, pdf] = sampleS1();
            accum[0] += func(x) / pdf;
        }
        {
            auto const [x, pdf] = sampleS2();
            accum[1] += func(x) / pdf;
        }
        {
            auto const [x, pdf] = sampleS3();
            accum[2] += func(x) / pdf;
        }
    }
    for(float const a : accum)
        std::cout << a / N << std::endl;
}
