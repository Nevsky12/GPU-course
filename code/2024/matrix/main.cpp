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


template<typename T>
Matrix<T> multiplyHitro(Matrix<T> const &m1, Matrix<T> const &m2, std::size_t const threadNo) noexcept
{
    assert(m1.width == m2.height);
    
    ThreadPool pool(threadNo);

    Matrix<T> m = emptyMatrix<T>(m2.width, m1.height);

    std::size_t const M = m1.height;
    std::size_t const K = m1.width ;
    std::size_t const N = m2.width ;

    std::size_t const L1 =   32u * 1024u;
    std::size_t const L2 =  256u * 1024u;
    std::size_t const L3 = 2048u * 1024u;

    std::size_t const mK = std::min(L1 / 4 / 16, K) / 4 * 4;
    std::size_t const mM = std::min(L2 / 4 / mK, M) / 8 * 8;
    std::size_t const mN = std::min(L3 / 4 / mK, N) / 16 * 16;
 
    std::cout << "mK mM mN : " << mK << " " << mM << " " << mN << std::endl;

    for(std::size_t i = 0u; i < N; i += mN)
    for(std::size_t j = 0u; j < K; j += mK)
    for(std::size_t k = 0u; k < M; k += mM)
    {
        pool.addTaskSeeFuture
        (
            [&] noexcept
            {
                for(std::size_t ii = i; ii < std::min(i + mN, N); ++ii)
                for(std::size_t jj = j; jj < std::min(j + mK, K); ++jj)
                {
                    f32 cV = 0.f;
                    for (std::size_t kk = k; kk < std::min(k + mM, M); ++kk) 
                    { 
        std::cout << "i j k : " << i << " " << j << " " << k << std::endl;
                 //       std::cout << "ii jj kk : " << ii << " " << jj << " " << kk << std::endl;
                        __m128 a_block = _mm_loadu_ps(m1[kk] + jj);
                        __m128 b_block = _mm_loadu_ps(m2[jj] + ii);

                        cV += _mm_cvtss_f32(_mm_fmadd_ps(a_block, b_block, _mm_setzero_ps()));
                    }
                    std::atomic_ref<float> cAR(m[ii][jj]);
                    cAR.store(cV, std::memory_order_relaxed);
                }
            }
        );
    }

    pool.wait();
    return m;
}

template<typename T>
void printMatrix(Matrix<T> const &m) noexcept
{
    for(std::size_t i = 0u; i < m.height; ++i)
    {
        for(std::size_t j = 0u; j < m.width; ++j)
        {
            std::cout << m[i][j] << ", ";
        }
        std::cout << std::endl;
    }
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
        std::size_t const n = 16;
        std::size_t const N = n;
        std::size_t const M = n;
        std::size_t const K = n;

        Matrix<float> m1 = emptyMatrix<float>(M, N, align);
        Matrix<float> m2 = emptyMatrix<float>(K, M, align);

        for(std::size_t i = 0u; i < N; ++i)
        for(std::size_t j = 0u; j < M; ++j)
            m1[i][j] = 1; //float(i * m1.width + j);

        for(std::size_t i = 0u; i < M; ++i)
        for(std::size_t j = 0u; j < K; ++j)
            m2[i][j] = 1; //float(i * m2.width + j);

        //printMatrix(m1); down
        //printMatrix(m2); down

        Matrix<float> m;
        auto const [E, D] = utils::stats<4u>([&]() noexcept
        {
            //m = multiplyReordered(m1, m2);
            m = multiplyHitro(m1, m2, 16);
        });
        printMatrix(m);

        std::cout << n << " " << 1000. * E << " " << 1000. * std::sqrt(D) << std::endl;
    //}

    return 0;
}
