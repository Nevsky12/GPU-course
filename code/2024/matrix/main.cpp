#include "tools/threadpool.h"
#include "tools/simd.h"
#include "tools/stats.h"
#include "tools/matrix.h"

#include <atomic>

using vf32 = stdx::native_simd<f32>;


template<typename T>
Matrix<T> multiplyReordered(Matrix<T> const &m1, Matrix<T> const &m2) noexcept
{
    assert(m1.width == m2.height);

    Matrix<T> m = emptyMatrix<T>(m2.width, m1.height);
    for(std::size_t i = 0u; i < m1.height; ++i)
        for(std::size_t k = 0u; k < m1.width; ++k)
            for(std::size_t j = 0u; j < m2.width; ++j)
                m[i][j] += m1[i][k] * m2[k][j];
    return m;
}

f32 * emptyMatrix( std::size_t const M
                 , std::size_t const N
                 , std::size_t const align
                 ) noexcept
{
    f32 * res = static_cast<f32 *>(std::malloc(M * N * sizeof(f32)));
    return res;
}


void printMatrix(f32 * m, std::size_t const M, std::size_t const N) noexcept
{
    for(std::size_t i = 0u; i < M; ++i)
    {
        for(std::size_t j = 0u; j < N; ++j)
        {
            std::cout << m[i * M + j] << ", ";
        }
        std::cout << std::endl;
    }
}


f32 * multiplyHitro( f32 * m1
                       , f32 * m2
                       , std::size_t const M
                       , std::size_t const N
                       , std::size_t const K
                       , std::size_t const threadNo
                       , std::size_t const align
                       ) noexcept
{
//    ThreadPool pool(threadNo);

    f32 * m = emptyMatrix(M, N, align);

    std::size_t const L1 =   32u * 1024u;
    std::size_t const L2 =  1000u * 1024u;
    std::size_t const L3 = 2048u * 1024u;

    std::size_t const mK = std::min(L1 / 4 / 16, K) / 4 * 4;
    std::size_t const mM = std::min(L2 / 4 / mK, M) / 8 * 8;
    std::size_t const mN = std::min(L3 / 4 / mK, N) / 16 * 16;
 
    std::size_t const rs = vf32::size();

    std::cout << "mK mM mN : " << mK << " " << mM << " " << mN << std::endl;

    for(std::size_t i = 0u; i < M; i += mM)
    for(std::size_t j = 0u; j < N; j += mN)
    for(std::size_t k = 0u; k < K; k += mK)
    {
        std::size_t const dM = std::min(i + mM, M);
        std::size_t const dN = std::min(j + mN, N);
        std::size_t const dK = std::min(k + mK, K);

        /*
        pool.addTaskSeeFuture
        (
            [&] noexcept
            {
            */
                for(std::size_t ii = i; ii < dM; ++ii)
                for(std::size_t kk = k; kk < dK; ++kk)
                {
                    f32 acc = 0.f;
                    for(std::size_t jj = j; jj < dN; jj += rs)
                    {
                        vf32 const kA(m1 + ii * dM + jj, stdx:: vector_aligned);
                        vf32 const kB(m2 + jj * dK + kk, stdx::element_aligned);
                        acc += stdx::reduce(kA * kB);
                    }
                    //std::atomic_ref<float> cAR(m[ii * dM + kk]);
                    //cAR.store(acc, std::memory_order_relaxed);
                    m[ii * M + kk] = acc;
                }
                /*
            }
        );
        */
    }


    /*
    for(std::size_t k = 0u; k < M / mM; k += mM)
    for(std::size_t i = 0u; i < N / mN; i += mN)
    for(std::size_t j = 0u; j < K / mK; j += mK)
    {
        pool.addTaskSeeFuture
        (
            [&] noexcept
            {
                std::size_t const dM = std::min(k + mM, M);
                std::size_t const dN = std::min(i + mN, N);
                std::size_t const dK = std::min(j + mK, K);

                for(std::size_t kk = k; kk < std::min(k + mM, M); ++kk)
                for(std::size_t ii = i; ii < std::min(i + mN, N); ++ii)
                {
                    f32 cV = 0.f;
                    for (std::size_t jj = j; jj < std::min(j + mK, K); ++jj) 
                    { 
                        std::cout << "i j k : " << i << " " << j << " " << k << std::endl;
                        std::cout << "ii jj kk : " << ii << " " << jj << " " << kk << std::endl;
                        std::cout << std::endl;
                        vf32 kA, kB;
                        kA.copy_from(m1 + kk * dM + jj, stdx::vector_aligned);
                        kB.copy_from(m2 + jj * dK + ii, stdx::vector_aligned);
                        __m128 a_block = _mm_loadu_ps(m1 + kk * dM + jj);
                        __m128 b_block = _mm_loadu_ps(m2 + jj * dK + ii);
                        cV += _mm_cvtss_f32(_mm_fmadd_ps(a_block, b_block, _mm_setzero_ps()));
                        cV += stdx::reduce(kA * kB);
                    }
                    std::atomic_ref<float> cAR(m[kk * dM + ii]);
                    cAR.store(cV, std::memory_order_relaxed);
                }
            }
        );
    }
    */

//    pool.wait();
    return m;
}


#define down \
    std::cout << std::endl;

int main()
{
    std::size_t const align = 64u;

    /*
    for(std::size_t n = 16; n <= 2048; n += n / 4)
    {
    */
        std::size_t const n = 1280;
        std::size_t const N = n;
        std::size_t const M = n;
        std::size_t const K = n;

        f32 * const m1 = emptyMatrix(M, K, align);
        f32 * const m2 = emptyMatrix(K, N, align);
        //Matrix<float> m1 = emptyMatrix<float>(M, N, align);
        //Matrix<float> m2 = emptyMatrix<float>(K, M, align);

        for(std::size_t i = 0u; i < M; ++i)
        for(std::size_t j = 0u; j < K; ++j)
            m1[i * M + j] = 1; //float(i * m1.width + j);

        for(std::size_t i = 0u; i < K; ++i)
        for(std::size_t j = 0u; j < N; ++j)
            m2[i * K + j] = 1; //float(i * m2.width + j);

        //printMatrix(m1); down
        //printMatrix(m2); down

        f32 * m;
        auto const [E, D] = utils::stats<4u>([&]() noexcept
        {
            //m = multiplyReordered(m1, m2);
            m = multiplyHitro(m1, m2, M, N, K, 16, align);
        });
        printMatrix(m, M, N);

        std::cout << n << " " << 1000. * E << " " << 1000. * std::sqrt(D) << std::endl;
    //}

    std::free(m1);
    std::free(m2);
    std::free(m );
    return 0;
}
