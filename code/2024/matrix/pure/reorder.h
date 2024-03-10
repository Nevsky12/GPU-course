void reorder_b_16(int K, const float * B, int ldb, float * bufB)
{
    for (int k = 0; k < K; ++k, B += ldb, bufB += 16)
    {
        _mm256_storeu_ps(bufB + 0, _mm256_loadu_ps(B + 0));
        _mm256_storeu_ps(bufB + 8, _mm256_loadu_ps(B + 8));
    }
}


void reorder_a_6(const float * A, int lda, int M, int K, float * bufA)
{
    for (int i = 0; i < M; i += 6)
    {
        for (int k = 0; k < K; k += 4)
        {
            const float * pA = A + k;
            __m128 a0  = _mm_loadu_ps(pA + 0 * lda);
            __m128 a1  = _mm_loadu_ps(pA + 1 * lda);
            __m128 a2  = _mm_loadu_ps(pA + 2 * lda);
            __m128 a3  = _mm_loadu_ps(pA + 3 * lda);
            __m128 a4  = _mm_loadu_ps(pA + 4 * lda);
            __m128 a5  = _mm_loadu_ps(pA + 5 * lda);

            __m128 a00 = _mm_unpacklo_ps(a0, a2);
            __m128 a01 = _mm_unpacklo_ps(a1, a3);
            __m128 a10 = _mm_unpackhi_ps(a0, a2);
            __m128 a11 = _mm_unpackhi_ps(a1, a3);
            __m128 a20 = _mm_unpacklo_ps(a4, a5);
            __m128 a21 = _mm_unpackhi_ps(a4, a5);

            _mm_storeu_ps(bufA + 0, _mm_unpacklo_ps(a00, a01));
            _mm_storel_pi((__m64*)(bufA + 4), a20);

            _mm_storeu_ps(bufA + 6, _mm_unpackhi_ps(a00, a01));
            _mm_storeh_pi((__m64*)(bufA + 10), a20);

            _mm_storeu_ps(bufA + 12, _mm_unpacklo_ps(a10, a11));
            _mm_storel_pi((__m64*)(bufA + 16), a21);

            _mm_storeu_ps(bufA + 18, _mm_unpackhi_ps(a10, a11));
            _mm_storeh_pi((__m64*)(bufA + 22), a21);
            
            bufA += 24;
        }
        A += 6 * lda;
    }
}
