#include <iostream>
#include <numbers>
#include <random>

struct Sample
{
    float x, y;
    float pdf;
};

float f(float x, float y)
{
    return std::abs(x);
}

int main()
{
    std::mt19937_64 gen(std::random_device{}());
    std::uniform_real_distribution<float> dist(-1.f, 1.f);

    auto const sample = [&]()
    {
        while(true)
        {
            float const x = dist(gen);
            float const y = dist(gen);
            if(x * x + y * y <= 1.f)
                return Sample{x, y, std::numbers::inv_pi_v<float>};
        }
    };

    auto const N = 1000u;
    float accum = 0.f;
    for(auto i = 0u; i < N; ++i)
    {
        auto const [x, y, pdf] = sample();
        accum += f(x, y) / pdf;
    }
    std::cout << accum / N << std::endl;
}
