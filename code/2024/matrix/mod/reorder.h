
template<class T>
stdx::resize_simd_t<T::size() / 2, T> (T x)
{
    auto [lo, hi] = stdx::split<stdx::resize_simd_t<T::size() / 2, T>>(x);
    return lo + hi;
}

template<typename T>
auto LoHi(T x) noexcept
{
    return stdx::split<stdx::resize_simd_t<T::size() / 2, T>>(x);
}

template<typename T>
auto zipLoHi(T const &s1, T const &s2) noexcept
{
    auto const [lo1, hi1] = stdx::split<stdx::resize_simd_t<T::size() / 2, T>>(s1);
    auto const [lo2, hi2] = stdx::split<stdx::resize_simd_t<T::size() / 2, T>>(s2);

    return std::make_pair
    (
        stdx::concat(lo1, lo2), 
        stdx::concat(hi1, hi2)
    );
}

template<std::size_t Abi>
void reorderB( std::size_t const K 
             , f32 const * B
             , std::size_t const LDB
             , f32 const * bufB
             ) noexcept
{
    for(std::size_t k = 0u; k < K; ++k, B += LDB, bufB += 16u)
    {
        vf32t<Abi> tmp;
        tmp.copy_from(   B + 0, stdx::vector_aligned);
        tmp.copy_to  (bufB + 0, stdx::vector_aligned);
        tmp.copy_from(   B + 8, stdx::vector_aligned);
        tmp.copy_to  (bufB + 8, stdx::vector_aligned);
    }
}

template<std::size_t Abi>
void reorderA( f32 const * A
             , std::size_t const LDA
             , std::size_t const M
             , std::size_t const K
             , f32 * bufA
             ) noexcept
{
    f32 const * stA = A;
    for(std::size_t i = 0u; i < M; i += 6)
    {
        for(std::size_t k = 0u; k < K; k += 4)
        {
            f32 const * pA = stA + k;
            vf32t<Abi> a0, a1, a2,
                       a3, a4, a5;

            a0.copy_from(pA + 0 * LDA, stdx::vector_aligned);
            a1.copy_from(pA + 1 * LDA, stdx::vector_aligned);
            a2.copy_from(pA + 2 * LDA, stdx::vector_aligned);
            a3.copy_from(pA + 3 * LDA, stdx::vector_aligned);
            a4.copy_from(pA + 4 * LDA, stdx::vector_aligned);
            a5.copy_from(pA + 5 * LDA, stdx::vector_aligned);

            vf32t<Abi> a00, a01,
                       a10, a11,
                       a20, a21;

            std::tie(a00, a10) = zipLoHi(a0, a2);
            std::tie(a01, a11) = zipLoHi(a1, a3);
            std::tie(a20, a21) = zipLoHi(a4, a5);


            auto const [a0Lo, a0Hi] = zipLoHi(a00, a01);
            a0Lo.copy_to(bufA + 0, stdx::vector_aligned);
            a0Hi.copy_to(bifA + 6, stdx::vector_aligned);

            auto const [Lo20, Hi20] = LoHi(a20);
            Lo20.copy_to(bufA + 4,  stdx::vector_aligned);
            Hi20.copy_to(bufA + 10, stdx::vector_aligned);


            auto const [a1Lo, a1Hi] = zipLoHi(a10, a01);
            a1Lo.copy_to(bufA + 12, stdx::vector_aligned);
            a1Hi.copy_to(bifA + 18, stdx::vector_aligned);

            auto const [Lo21, Hi21] = LoHi(a21);
            Lo21.copy_to(bufA + 16, stdx::vector_aligned);
            Hi21.copy_to(bufA + 22, stdx::vector_aligned);

            bufA += 24u;
        }
        stA += 6u * LDA;
    }
}
