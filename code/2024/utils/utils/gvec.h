#pragma once
#include <cassert>
#include <cstddef>
#include <cmath>

namespace utils
{

template<typename T, std::size_t N>
struct gvec 
{
    using type = T;
};

template<typename T>
struct gvec<T, 1>
{
    T x;

    using type = T;

    gvec() = default;
    constexpr gvec(T const &a) noexcept : x(a) {}
    constexpr gvec(gvec<T, 0> const &, T const &last) noexcept : x(last) {}
    constexpr gvec(gvec<T, 2> const &v) noexcept;
    template<typename U>
    constexpr gvec(gvec<U, 1> const &v) noexcept : x(T(v.x)) {}

    constexpr T const &operator[](std::size_t const i) const noexcept {assert(i < 1u); return (&x)[i];}
    constexpr T       &operator[](std::size_t const i)       noexcept {assert(i < 1u); return (&x)[i];}
};
template<typename T>
struct gvec<T, 2>
{
    T x, y;

    using type = T;

    gvec() = default;
    constexpr gvec(T const &a) noexcept : x(a), y(a) {}
    constexpr gvec(T const &a, T const &b) noexcept : x(a), y(b) {}
    constexpr gvec(gvec<T, 1> const &init, T const &last) noexcept : x(init.x), y(last) {}
    constexpr gvec(gvec<T, 3> const &v) noexcept;
    template<typename U>
    constexpr gvec(gvec<U, 2> const &v) noexcept : x(T(v.x)), y(T(v.y)) {}

    constexpr T const &operator[](std::size_t const i) const noexcept {assert(i < 2u); return (&x)[i];}
    constexpr T       &operator[](std::size_t const i)       noexcept {assert(i < 2u); return (&x)[i];}
};
template<typename T>
struct gvec<T, 3>
{
    T x, y, z;

    using type = T;

    gvec() = default;
    constexpr gvec(T const &a) noexcept : x(a), y(a), z(a) {}
    constexpr gvec(T const &a, T const &b, T const &c) noexcept : x(a), y(b), z(c) {}
    constexpr gvec(gvec<T, 2> const &init, T const &last) noexcept : x(init.x), y(init.y), z(last) {}
    constexpr gvec(gvec<T, 4> const &v) noexcept;
    template<typename U>
    constexpr gvec(gvec<U, 3> const &v) noexcept : x(T(v.x)), y(T(v.y)), z(T(v.z)) {}

    constexpr T const &operator[](std::size_t const i) const noexcept {assert(i < 3u); return (&x)[i];}
    constexpr T       &operator[](std::size_t const i)       noexcept {assert(i < 3u); return (&x)[i];}
};
template<typename T>
struct gvec<T, 4>
{
    T x, y, z, w;

    using type = T;

    gvec() = default;
    constexpr gvec(T const &a) noexcept : x(a), y(a), z(a), w(a) {}
    constexpr gvec(T const &a, T const &b, T const &c, T const &d) noexcept : x(a), y(b), z(c), w(d) {}
    constexpr gvec(gvec<T, 3> const &init, T const &last) noexcept : x(init.x), y(init.y), z(init.z), w(last) {}
    template<typename U>
    constexpr gvec(gvec<U, 4> const &v) noexcept : x(T(v.x)), y(T(v.y)), z(T(v.z)), w(T(v.w)) {}

    constexpr T const &operator[](std::size_t const i) const noexcept {assert(i < 4u); return (&x)[i];}
    constexpr T       &operator[](std::size_t const i)       noexcept {assert(i < 4u); return (&x)[i];}
};

template<typename T>
constexpr gvec<T, 0> init(gvec<T, 1> const & ) noexcept {return {};}
template<typename T>
constexpr gvec<T, 1> init(gvec<T, 2> const &v) noexcept {return {v.x};}
template<typename T>
constexpr gvec<T, 2> init(gvec<T, 3> const &v) noexcept {return {v.x, v.y};}
template<typename T>
constexpr gvec<T, 3> init(gvec<T, 4> const &v) noexcept {return {v.x, v.y, v.z};}

template<typename T>
constexpr T const &  last(gvec<T, 1> const &v) noexcept {return v.x;}
template<typename T>
constexpr T const &  last(gvec<T, 2> const &v) noexcept {return v.y;}
template<typename T>
constexpr T const &  last(gvec<T, 3> const &v) noexcept {return v.z;}
template<typename T>
constexpr T const &  last(gvec<T, 4> const &v) noexcept {return v.w;}

template<typename T>
constexpr gvec<T, 2>::gvec(gvec<T, 3> const &v) noexcept : x(v.x), y(v.y) {}
template<typename T>
constexpr gvec<T, 3>::gvec(gvec<T, 4> const &v) noexcept : x(v.x), y(v.y), z(v.z) {}

template<typename T, std::size_t N, std::size_t M>
using gmat = gvec<gvec<T, M>, N>; // column-major order: N columns, M rows, indexing: m[column][row]

template<typename T, std::size_t N>
constexpr T       *begin(gvec<T, N>       &v) noexcept {return &v.x;}
template<typename T, std::size_t N>
constexpr T       *  end(gvec<T, N>       &v) noexcept {return &v.x + N;}
template<typename T, std::size_t N>
constexpr T const *begin(gvec<T, N> const &v) noexcept {return &v.x;}
template<typename T, std::size_t N>
constexpr T const *  end(gvec<T, N> const &v) noexcept {return &v.x + N;}

template<typename F, typename T>
constexpr T foldl1(F const &func, gvec<T, 2> const &v) noexcept {return func(v.x, v.y);}
template<typename F, typename T>
constexpr T foldl1(F const &func, gvec<T, 3> const &v) noexcept {return func(func(v.x, v.y), v.z);}
template<typename F, typename T>
constexpr T foldl1(F const &func, gvec<T, 4> const &v) noexcept {return func(func(func(v.x, v.y), v.z), v.w);}

template<typename F, typename T>
constexpr T foldr1(F const &func, gvec<T, 2> const &v) noexcept {return func(v.x, v.y);}
template<typename F, typename T>
constexpr T foldr1(F const &func, gvec<T, 3> const &v) noexcept {return func(v.x, func(v.y, v.z));}
template<typename F, typename T>
constexpr T foldr1(F const &func, gvec<T, 4> const &v) noexcept {return func(v.x, func(v.y, func(v.z, v.w)));}

template<typename F, typename T>
constexpr auto liftA1(F const &func, gvec<T, 2> const &v) noexcept
{
    return gvec<decltype(func(v.x)), 2>
    {
        func(v.x),
        func(v.y),
    };
}
template<typename F, typename T>
constexpr auto liftA1(F const &func, gvec<T, 3> const &v) noexcept
{
    return gvec<decltype(func(v.x)), 3>
    {
        func(v.x),
        func(v.y),
        func(v.z),
    };
}
template<typename F, typename T>
constexpr auto liftA1(F const &func, gvec<T, 4> const &v) noexcept
{
    return gvec<decltype(func(v.x)), 4>
    {
        func(v.x),
        func(v.y),
        func(v.z),
        func(v.w),
    };
}
template<typename F>
constexpr auto transformV(F const &func) noexcept
{
    return [func](auto const &v) noexcept {return liftA1(func, v);};
}

template<typename F, typename T>
constexpr auto liftA2(F const &func, gvec<T, 2> const &v1, gvec<T, 2> const &v2) noexcept
{
    return gvec<decltype(func(v1.x, v2.x)), 2>
    {
        func(v1.x, v2.x),
        func(v1.y, v2.y),
    };
}
template<typename F, typename T>
constexpr auto liftA2(F const &func, gvec<T, 3> const &v1, gvec<T, 3> const &v2) noexcept
{
    return gvec<decltype(func(v1.x, v2.x)), 3>
    {
        func(v1.x, v2.x),
        func(v1.y, v2.y),
        func(v1.z, v2.z),
    };
}
template<typename F, typename T>
constexpr auto liftA2(F const &func, gvec<T, 4> const &v1, gvec<T, 4> const &v2) noexcept
{
    return gvec<decltype(func(v1.x, v2.x)), 4>
    {
        func(v1.x, v2.x),
        func(v1.y, v2.y),
        func(v1.z, v2.z),
        func(v1.w, v2.w),
    };
}

#define lambdaE1(x,    f) [ ](auto &&x          ) noexcept {return f;}
#define lambdaE2(x, y, f) [ ](auto &&x, auto &&y) noexcept {return f;}
#define lambdaR1(x,    f) [&](auto &&x          ) noexcept {return f;}
#define lambdaR2(x, y, f) [&](auto &&x, auto &&y) noexcept {return f;}

#define lambdaAE1(x,    f) [ ](auto &&x          ) -> decltype(auto) noexcept {return f;}
#define lambdaAE2(x, y, f) [ ](auto &&x, auto &&y) -> decltype(auto) noexcept {return f;}
#define lambdaAR1(x,    f) [&](auto &&x          ) -> decltype(auto) noexcept {return f;}
#define lambdaAR2(x, y, f) [&](auto &&x, auto &&y) -> decltype(auto) noexcept {return f;}

template<typename T, std::size_t N>
constexpr gvec<T, N> operator+(gvec<T, N> const &v1, gvec<T, N> const &v2) noexcept
{
    return liftA2(lambdaE2(x, y, x + y), v1, v2);
}
template<typename T, std::size_t N>
constexpr gvec<T, N> operator-(gvec<T, N> const &v1, gvec<T, N> const &v2) noexcept
{
    return liftA2(lambdaE2(x, y, static_cast<T>(x - y)), v1, v2);
}
template<typename T, std::size_t N>
constexpr gvec<T, N> operator-(gvec<T, N> const &v) noexcept
{
    return liftA1(lambdaE1(x, -x), v);
}
template<typename T, std::size_t N>
constexpr gvec<T, N> operator*(gvec<T, N> const &v1, gvec<T, N> const &v2) noexcept
{
    return liftA2(lambdaE2(x, y, x * y), v1, v2);
}
template<typename T, std::size_t N>
constexpr gvec<T, N> operator/(gvec<T, N> const &v1, gvec<T, N> const &v2) noexcept
{
    return liftA2(lambdaE2(x, y, x / y), v1, v2);
}

template<typename T, std::size_t N>
constexpr gvec<T, N> &operator+=(gvec<T, N> &v1, gvec<T, N> const &v2) noexcept {return v1 = v1 + v2;}
template<typename T, std::size_t N>
constexpr gvec<T, N> &operator-=(gvec<T, N> &v1, gvec<T, N> const &v2) noexcept {return v1 = v1 - v2;}
template<typename T, std::size_t N>
constexpr gvec<T, N> &operator*=(gvec<T, N> &v1, gvec<T, N> const &v2) noexcept {return v1 = v1 * v2;}
template<typename T, std::size_t N>
constexpr gvec<T, N> &operator/=(gvec<T, N> &v1, gvec<T, N> const &v2) noexcept {return v1 = v1 / v2;}

template<typename T, std::size_t N>
constexpr T dot(gvec<T, N> const &v1, gvec<T, N> const &v2)
{
    return foldl1(lambdaE2(x, y, x + y), v1 * v2);
}
template<typename T>
constexpr gvec<T, 3> cross(gvec<T, 3> const &v1, gvec<T, 3> const &v2) noexcept
{
    return
    {
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x,
    };
}
template<typename T, std::size_t N>
auto length(gvec<T, N> const &v) noexcept
{
    return std::sqrt(dot(v, v));
}
template<typename T, std::size_t N>
gvec<T, N> normalize(gvec<T, N> const &v) noexcept
{
    return v / gvec<T, N>(length(v));
}

template<typename T, std::size_t N>
constexpr auto operator< (gvec<T, N> const &v1, gvec<T, N> const &v2) noexcept
{
    return liftA2(lambdaE2(x, y, x < y), v1, v2);
}
template<typename T, std::size_t N>
constexpr auto operator<=(gvec<T, N> const &v1, gvec<T, N> const &v2) noexcept
{
    return liftA2(lambdaE2(x, y, x <= y), v1, v2);
}
template<typename T, std::size_t N>
constexpr auto operator> (gvec<T, N> const &v1, gvec<T, N> const &v2) noexcept
{
    return liftA2(lambdaE2(x, y, x >  y), v1, v2);
}
template<typename T, std::size_t N>
constexpr auto operator>=(gvec<T, N> const &v1, gvec<T, N> const &v2) noexcept
{
    return liftA2(lambdaE2(x, y, x >= y), v1, v2);
}
template<typename T, std::size_t N>
constexpr auto operator==(gvec<T, N> const &v1, gvec<T, N> const &v2) noexcept
{
    return liftA2(lambdaE2(x, y, x == y), v1, v2);
}
template<typename T, std::size_t N>
constexpr auto operator!=(gvec<T, N> const &v1, gvec<T, N> const &v2) noexcept
{
    return liftA2(lambdaE2(x, y, x != y), v1, v2);
}
template<std::size_t N>
constexpr gvec<bool, N> operator not(gvec<bool, N> const &v) noexcept {return liftA1(lambdaE1(x, not x), v);}
template<std::size_t N>
constexpr bool any(gvec<bool, N> const &v) noexcept {return foldl1(lambdaE2(x, y, x || y), v);}
template<std::size_t N>
constexpr bool all(gvec<bool, N> const &v) noexcept {return foldl1(lambdaE2(x, y, x && y), v);}

template<typename T>
T floor(T const &x) noexcept
{
    return std::floor(x);
}
template<typename T, std::size_t N>
gvec<T, N> floor(gvec<T, N> const &v) noexcept
{
    return liftA1(utils::floor<T>, v);
}
template<typename T>
T ceil(T const &x) noexcept
{
    return std::ceil(x);
}
template<typename T, std::size_t N>
gvec<T, N> ceil(gvec<T, N> const &v) noexcept
{
    return liftA1(utils::ceil<T>, v);
}
template<typename T>
T round(T const &x) noexcept
{
    return std::round(x);
}
template<typename T, std::size_t N>
gvec<T, N> round(gvec<T, N> const &v) noexcept
{
    return liftA1(utils::round<T>, v);
}

template<typename T>
T fract(T const &x) noexcept
{
    return x - utils::floor(x);
}
template<typename T>
T mod(T const &x, T const &y) noexcept
{
    return x - y * utils::floor(x / y);
}

template<typename T>
constexpr T abs(T const &x) noexcept
{
    return x >= T(0) ? x : -x;
}
template<typename T, std::size_t N>
constexpr gvec<T, N> abs(gvec<T, N> const &v) noexcept
{
    return liftA1(utils::abs<T>, v);
}

// min and max will return b if (a == b) or (a or b is nan):
template<typename T>
constexpr T const &min(T const &a, T const &b) noexcept
{
    return a <  b ? a : b;
}
template<typename T>
constexpr T const &max(T const &a, T const &b) noexcept
{
    return a >= b ? a : b;
}
template<typename T, std::size_t N>
constexpr gvec<T, N> min(gvec<T, N> const &v1, gvec<T, N> const &v2) noexcept
{
    return liftA2(utils::min<T>, v1, v2);
}
template<typename T, std::size_t N>
constexpr gvec<T, N> max(gvec<T, N> const &v1, gvec<T, N> const &v2) noexcept
{
    return liftA2(utils::max<T>, v1, v2);
}
template<typename T>
constexpr T clamp(T const &x, T const &min, T const &max) noexcept
{
    return utils::max(min, utils::min(max, x));
}

template<std::size_t N, std::size_t M>
constexpr bool any(gmat<bool, N, M> const &m) noexcept {return any(liftA1(lambdaE1(v, any(v)), m));}
template<std::size_t N, std::size_t M>
constexpr bool all(gmat<bool, N, M> const &m) noexcept {return all(liftA1(lambdaE1(v, all(v)), m));}

template<typename T, std::size_t N>
constexpr gmat<T, N, N> fromDiagonal(gvec<T, N> const &v) noexcept
{
    if constexpr(N == 2)
        return
        {
            {v.x , T(0)},
            {T(0), v.y },
        };
    if constexpr(N == 3)
        return
        {
            {v.x , T(0), T(0)},
            {T(0), v.y , T(0)},
            {T(0), T(0), v.z },
        };
    if constexpr(N == 4)
        return
        {
            {v.x , T(0), T(0), T(0)},
            {T(0), v.y , T(0), T(0)},
            {T(0), T(0), v.z , T(0)},
            {T(0), T(0), T(0), v.w },
        };
}
template<typename T, std::size_t N>
constexpr auto identity = fromDiagonal(gvec<T, N>(T(1)));

template<typename T, std::size_t N, std::size_t M>
constexpr gmat<T, M, N> transpose(gmat<T, N, M> const &m) noexcept
{
    return
    {
        transpose(liftA1(lambdaE1(v, init(v)), m)),
                  liftA1(lambdaE1(v, last(v)), m) ,
    };
}
template<typename T, std::size_t N>
constexpr gmat<T, 0, N> transpose(gmat<T, N, 0> const & ) noexcept
{
    return {};
}

template<typename T, std::size_t N, std::size_t M, std::size_t K>
constexpr gmat<T, K, N> compose(gmat<T, M, N> const &m1, gmat<T, K, M> const &m2) noexcept
{
    return transpose( liftA1( lambdaR1 ( row
                                       , liftA1( lambdaR1( column
                                                         , dot(row, column)
                                                         )
                                               , m2
                                               )
                                       )
                            , transpose(m1)
                            )
                    );
}

#if __cplusplus >= 201907L // if supports concepts:
namespace detail
{
template<typename T>
concept regular = requires(T const a, T const b) {bool(a == b);};
} // namespace detail

template<detail::regular T, std::size_t N>
constexpr gvec<T, N> operator*(T const &x, gvec<T, N> const &v) noexcept
{
    return gvec<T, N>(x) * v;
}
template<detail::regular T, std::size_t N>
constexpr gvec<T, N> operator*(gvec<T, N> const &v, T const &x) noexcept
{
    return v * gvec<T, N>(x);
}
template<detail::regular T, std::size_t N>
constexpr gvec<T, N> operator/(T const &x, gvec<T, N> const &v) noexcept
{
    return gvec<T, N>(x) / v;
}
template<detail::regular T, std::size_t N>
constexpr gvec<T, N> operator/(gvec<T, N> const &v, T const &x) noexcept
{
    return v / gvec<T, N>(x);
}
template<detail::regular T, std::size_t N, std::size_t M>
constexpr gmat<T, N, M> operator*(T const &x, gmat<T, N, M> const &m) noexcept
{
    return gmat<T, N, M>(gvec<T, M>(x)) * m;
}
template<detail::regular T, std::size_t N, std::size_t M>
constexpr gmat<T, N, M> operator*(gmat<T, N, M> const &m, T const &x) noexcept
{
    return m * gmat<T, N, M>(gvec<T, M>(x));
}
template<detail::regular T, std::size_t N, std::size_t M>
constexpr gmat<T, N, M> operator/(gmat<T, N, M> const &m, T const &x) noexcept
{
    return m / gmat<T, N, M>(gvec<T, M>(x));
}

template<detail::regular T, std::size_t N>
constexpr gvec<T, N> &operator*=(gvec<T, N> &v, T const &x) noexcept {return v = v * x;}
template<detail::regular T, std::size_t N>
constexpr gvec<T, N> &operator/=(gvec<T, N> &v, T const &x) noexcept {return v = v / x;}
template<detail::regular T, std::size_t N, std::size_t M>
constexpr gmat<T, N, M> &operator*=(gmat<T, N, M> &m, T const &x) noexcept {return m = m * x;}
template<detail::regular T, std::size_t N, std::size_t M>
constexpr gmat<T, N, M> &operator/=(gmat<T, N, M> &m, T const &x) noexcept {return m = m / x;}

template<detail::regular T, std::size_t N, std::size_t M, std::size_t K>
constexpr gmat<T, K, N> operator*(gmat<T, M, N> const &m1, gmat<T, K, M> const &m2) noexcept
{
    return compose(m1, m2);
}
template<detail::regular T, std::size_t N>
constexpr gmat<T, N, N> operator*(gmat<T, N, N> const &m1, gmat<T, N, N> const &m2) noexcept
{
    return compose(m1, m2);
}
#endif

template<typename T, std::size_t N, std::size_t M>
constexpr gvec<T, M> operator*(gmat<T, N, M> const &m, gvec<T, N> const &v) noexcept
{
    return liftA1(lambdaR1(row, dot(row, v)), transpose(m));
}

template<typename T>
constexpr T determinant(gmat<T, 2, 2> const &m) noexcept
{
    return m[0][0] * m[1][1]
         - m[1][0] * m[0][1];
}
template<typename T>
constexpr T determinant(gmat<T, 3, 3> const &m) noexcept
{
    return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
         + m[0][1] * (m[1][2] * m[2][0] - m[1][0] * m[2][2])
         + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}
template<typename T>
constexpr T determinant(gmat<T, 4, 4> const &m) noexcept
{
    return m[0][0] * ( m[1][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2])
                     + m[1][2] * (m[2][3] * m[3][1] - m[2][1] * m[3][3])
                     + m[1][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1])
                     )
         + m[0][1] * ( m[1][2] * (m[2][0] * m[3][3] - m[2][3] * m[3][0])
                     + m[1][3] * (m[2][2] * m[3][0] - m[2][0] * m[3][2])
                     + m[1][0] * (m[2][3] * m[3][2] - m[2][2] * m[3][3])
                     )
         + m[0][2] * ( m[1][3] * (m[2][0] * m[3][1] - m[2][1] * m[3][0])
                     + m[1][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1])
                     + m[1][1] * (m[2][3] * m[3][0] - m[2][0] * m[3][3])
                     )
         + m[0][3] * ( m[1][0] * (m[2][2] * m[3][1] - m[2][1] * m[3][2])
                     + m[1][1] * (m[2][0] * m[3][2] - m[2][2] * m[3][0])
                     + m[1][2] * (m[2][1] * m[3][0] - m[2][0] * m[3][1])
                     );
}
template<typename T>
constexpr gmat<T, 2, 2> inverse(gmat<T, 2, 2> const &m) noexcept
{
    return gmat<T, 2, 2>
    {
        { m[1][1], -m[0][1]},
        {-m[1][0],  m[0][0]},
    } / gmat<T, 2, 2>(gvec<T, 2>(determinant(m)));
}
template<typename T>
constexpr gmat<T, 3, 3> inverse(gmat<T, 3, 3> const &m) noexcept
{
    auto const det2 = [&m](std::size_t const i, std::size_t const j) noexcept
    {
        auto const i1 = (i + 1u) % 3u;
        auto const j1 = (j + 1u) % 3u;
        auto const i2 = (i + 2u) % 3u;
        auto const j2 = (j + 2u) % 3u;
        return m[i1][j1] * m[i2][j2] - m[i1][j2] * m[i2][j1];
    };
    return gmat<T, 3, 3>
    {
        {det2(0, 0), det2(1, 0), det2(2, 0)},
        {det2(0, 1), det2(1, 1), det2(2, 1)},
        {det2(0, 2), det2(1, 2), det2(2, 2)},
    } / gmat<T, 3, 3>(gvec<T, 3>(determinant(m)));
}
template<typename T>
constexpr gmat<T, 4, 4> inverse(gmat<T, 4, 4> const &m) noexcept
{
    auto const det3 = [&m](std::size_t const i, std::size_t const j) noexcept
    {
        auto const i1 = (i + 1u) % 4u;
        auto const j1 = (j + 1u) % 4u;
        auto const i2 = (i + 2u) % 4u;
        auto const j2 = (j + 2u) % 4u;
        auto const i3 = (i + 3u) % 4u;
        auto const j3 = (j + 3u) % 4u;
        auto const d = m[i1][j1] * (m[i2][j2] * m[i3][j3] - m[i2][j3] * m[i3][j2])
                     + m[i1][j2] * (m[i2][j3] * m[i3][j1] - m[i2][j1] * m[i3][j3])
                     + m[i1][j3] * (m[i2][j1] * m[i3][j2] - m[i2][j2] * m[i3][j1]);
        return (i + j) % 2 == 0 ? d : -d;
    };
    return gmat<T, 4, 4>
    {
        {det3(0, 0), det3(1, 0), det3(2, 0), det3(3, 0)},
        {det3(0, 1), det3(1, 1), det3(2, 1), det3(3, 1)},
        {det3(0, 2), det3(1, 2), det3(2, 2), det3(3, 2)},
        {det3(0, 3), det3(1, 3), det3(2, 3), det3(3, 3)},
    } / gmat<T, 4, 4>(gvec<T, 4>(determinant(m)));
}

template<typename T>
constexpr gmat<T, 3, 3> fromQuaternion(gvec<T, 4> const q) noexcept
{
    return
    {
        {T(1) - T(2) * (q.y * q.y + q.z * q.z),        T(2) * (q.x * q.y + q.z * q.w),        T(2) * (q.x * q.z - q.y * q.w)},
        {       T(2) * (q.x * q.y - q.z * q.w), T(1) - T(2) * (q.x * q.x + q.z * q.z),        T(2) * (q.y * q.z + q.x * q.w)},
        {       T(2) * (q.x * q.z + q.y * q.w),        T(2) * (q.y * q.z - q.x * q.w), T(1) - T(2) * (q.x * q.x + q.y * q.y)},
    };
}

} // namespace utils
