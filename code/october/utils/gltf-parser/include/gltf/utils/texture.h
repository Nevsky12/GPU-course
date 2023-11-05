#pragma once
#include <gltf/utils/encode.h>
#include <gltf/utils/image.h>

namespace gltf::utils
{

template<typename T>
struct Texture
{
    Sampler         const &sampler;
    utils::Image<T> const &image;

    vec4 sample(vec2 const uv, bool const decodeSRGB = false) const noexcept
    {
        auto const fetch = [=, this](vec2 const p0) noexcept
            -> vec4
        {
            auto const f = [](i32 const x, i32 const n, Sampler::Wrap const w) noexcept
            {
                auto const mirror = [](i32 const a) noexcept {return a >= 0 ? a : -(1 + a);};
                switch(w)
                {
                    case Sampler::Wrap::   ClampToEdge: return clamp(x, 0, n - 1);
                    case Sampler::Wrap::        Repeat: return x % n;
                    case Sampler::Wrap::MirroredRepeat: return (n - 1) - mirror(x % (2 * n) - n);
                    default: assert(0); return 0;
                }
            };
            auto const [nx, ny] = image.extent();
            ivec2 const p =
            {
                f(i32(round(p0.x)), nx, sampler.wrapS),
                f(i32(round(p0.y)), ny, sampler.wrapT),
            };

            auto const c = image.fetch(p);
            if constexpr(sizeof(T) == 1)
                return decodeSRGB
                    ? vec4{liftA1(utils::decodeSRGB, gvec<u8, 3>(c)), utils::fromUnorm<8>(c.w)}
                    : liftA1(utils::fromUnorm<8>, c);
            if constexpr(sizeof(T) == 4)
                return c;
        };

        vec2 const p = uv * vec2(image.extent() - ivec2(1));
        Sampler::MagFilter const filter = sampler.magFilter.value_or(Sampler::MagFilter::Nearest);
        if(filter == Sampler::MagFilter::Nearest)
            return fetch(p);
        else
        {
            vec2 const pmin = floor(p);
            vec2 const pmax = pmin + vec2(1.f);
            gvec<vec2, 4> const p4 =
            {
                {pmin.x, pmin.y},
                {pmax.x, pmin.y},
                {pmin.x, pmax.y},
                {pmax.x, pmax.y},
            };
            vec4 const w =
            {
                (pmax.x - p.x) * (pmax.y - p.y),
                (p.x - pmin.x) * (pmax.y - p.y),
                (pmax.x - p.x) * (p.y - pmin.y),
                (p.x - pmin.x) * (p.y - pmin.y),
            };
            return liftA1(fetch, p4) * w;
        }
    }
};

} // namespace gltf::utils
