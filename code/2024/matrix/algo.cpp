#include "tools/threadpool.h"
#include "tools/simd.h"
#include "tools/stats.h"
#include "tools/matrix.h"

using vf32 = vf32t<8>;

f32 * alloc(std::size_t const n) noexcept
{
    f32 * ptr = (f32*) std::aligned_alloc(64u, 4u * n);
    memset(ptr, 0u, 4u * n); 
    return ptr;
}


template
<
    std::size_t RegPackSize,
    std::size_t RegSize
>
void kernel( f32  const * const a
           , vf32 const * const b
           , vf32       * const c
           , std::size_t const ii
           , std::size_t const jj
           , std::size_t const le
           , std::size_t const ri
           , std::size_t const N
           ) noexcept
{
    vf32 pack[RegPackSize][2u] = {0.f};

    for(std::size_t k = le; k < ri         ; ++k)
    for(std::size_t i = 0u; i < RegPackSize; ++i)
    {
        vf32 const ak = a[(ii + i) * N + k];
        for(std::size_t j = 0u; j < 2u; ++j)
            pack[i][j] += ak * b[(N * k + jj) / RegSize + j];
    }

    for(std::size_t i = 0u; i < RegPackSize; ++i)
    for(std::size_t j = 0u; j < 2u         ; ++j)
        c[((ii + i) * N  + jj) / RegSize + j] += pack[i][j];
}

constexpr std::size_t reserve = 1920u * 1920u; // ~16 MB

template
<
    std::size_t ProcessElemNo,
    std::size_t RegSize = vf32::size(),
    std::size_t Reg2Size    = RegSize * 2u,
    std::size_t RegPackSize = ProcessElemNo / Reg2Size
>
void matmul( f32 const * const A
           , f32 const * const B
           , f32       * const C
           , std::size_t const N
           ) noexcept
{
    std::size_t const Nx = (N + RegPackSize - 1u) / RegPackSize * RegPackSize;
    std::size_t const Ny = (N + Reg2Size    - 1u) / Reg2Size    * Reg2Size;

    alignas(64u) static f32 a[reserve]
                          , b[reserve]
                          , c[reserve];
    
    std::memset(c, 0u, sizeof(int) * Nx * Ny);

    for(std::size_t i = 0u; i < N; ++i)
    {
        std::memcpy(&a[i * Ny], &A[i * N], 4u * N);
        std::memcpy(&b[i * Ny], &B[i * N], 4u * N);
    }

    std::size_t const u = ProcessElemNo;
    std::size_t const s3 = u;
    std::size_t const s2 = 2u * u;
    std::size_t const s1 = 4u * u;

    ThreadPool pool(std::thread::hardware_concurrency());

    for(std::size_t i3 = 0u; i3 < Ny; i3 += s3)
    for(std::size_t i2 = 0u; i2 < Nx; i2 += s2)
    for(std::size_t i1 = 0u; i1 < Ny; i1 += s1)
        pool.enqueue([=] noexcept
        {
            for(std::size_t ii = i2; ii < std::min(i2 + s2, Nx); ii += RegPackSize)
            for(std::size_t jj = i3; jj < std::min(i3 + s3, Ny); jj += Reg2Size   )
                kernel<RegPackSize, RegSize>
                (
                    a, 
                    reinterpret_cast<vf32 const * const>(b), 
                    reinterpret_cast<vf32       * const>(c), 
                    
                    ii, 
                    jj, 
                    i1, 
                    std::min(i1 + s1, N), 
                    Ny
                );

            for(std::size_t i = 0u; i < N; ++i)
                std::memcpy(&C[i * N], &c[i * Ny], 4u * N);
        });
        
    pool.wait();
}

void printMatrix( f32 const * const m
                , std::size_t const M
                , std::size_t const N
                ) noexcept
{
    for(std::size_t i = 0u; i < M; ++i)
    {
        for(std::size_t j = 0u; j < N; ++j)
        {
            std::cout << m[i * M + j] << ", ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << std::endl;
}

f32 * emptyMatrix( std::size_t const M
                 , std::size_t const N
                 , std::size_t const align
                 ) noexcept
{
    f32 * res = static_cast<f32 *>(std::malloc(M * N * sizeof(f32)));
    return res;
}

int main()
{
    for(std::size_t n = 16u; n <= 1920u; n += 17u)
    {
        f32 * A = emptyMatrix(n, n, 64);
        f32 * B = emptyMatrix(n, n, 64);
        f32 * C = emptyMatrix(n, n, 64);

        std::size_t const M = n;
        std::size_t const K = n;
        std::size_t const N = n;
        for(std::size_t i = 0u; i < M; ++i)
        for(std::size_t j = 0u; j < K; ++j)
            A[i * M + j] = 1;
        for(std::size_t i = 0u; i < K; ++i)
        for(std::size_t j = 0u; j < N; ++j)
            B[i * K + j] = 1;

        auto const [E, D] = utils::stats<4u>([&] noexcept
        {
            matmul<96u>(A, B, C, n);
        });
   
        //printMatrix(C, M, N);
        std::cout << n << " " << 1000. * E << " " << 1000. * std::sqrt(D) << std::endl;
    }

    return 0;
}
