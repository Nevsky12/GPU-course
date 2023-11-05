#pragma once
#include <gltf/utils/ray.h>
namespace gltf::utils
{

using Triangle = gvec<vec3, 3>;

struct RayTriangleIntersection
{
    float t;
    vec2 barycentrics;
    bool frontFacing;
};

const RayTriangleIntersection rayTriangleMiss = {0.f, vec2(-1.f), false};
constexpr inline bool happened(RayTriangleIntersection const i) noexcept
{
    auto const [p, q] = i.barycentrics;
    return (1.f - p - q) >= 0.f
        &&        p      >= 0.f
        &&            q  >= 0.f;
}
template<typename T>
constexpr T interpolate(gvec<T, 3> const &r, vec2 const barycentrics) noexcept
{
    auto const [p, q] = barycentrics;
    return (1.f - p - q) * r[0]
                + p      * r[1]
                    + q  * r[2];
}

inline RayTriangleIntersection rayTriangleIntersection( const Ray ray
                                                      , const Triangle tri
                                                      , const RayDistanceRange rdr
                                                      ) noexcept
{
    const gltf::vec3 a =  tri[1] - tri[0];
    const gltf::vec3 b =  tri[2] - tri[0];
    const gltf::vec3 c = ray.pos - tri[0];
    const gltf::vec3 d = -ray.dir;

    const float det0 = dot(d, cross(a, b));

    const RayTriangleIntersection hit =
    {
        dot(c, cross(a, b)) / det0,
        {
            dot(d, cross(c, b)) / det0,
            dot(d, cross(a, c)) / det0,
        },
        det0 > 0.f,
    };

    return contains(rdr, hit.t)
        ? hit
        : rayTriangleMiss;
}

} // namespace gltf::utils
