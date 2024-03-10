#pragma once
#include "tools/types.h"
#include "tools/matrix.h"

using T = f32;

struct Buf
{
    f32 * p;
    std::size_t n;

    Buf(std::size_t size) 
    : 
        n(size), 
        p(static_cast<f32 *>(std::aligned_alloc(64u, size * 4u))) 
    {}

    ~Buf() { std::free(p); }
};


// LDA - leading dimension for A

#include "reorder.h"
#include "micro.h"
#include "macro.h"

Matrix<T> multiply( Matrix<T> const &A
                  , Matrix<T> const &B
                //, ThreadPool & pool
                  ) noexcept
{
    std::size_t const M = A.height
                    , K = A.width
                    , N = B.width;

    assert(K == N);

    Matrix<T> C = emptyMatrix<T>(N, M, 64);
    
    constexpr int L1 =       32 * 1024
                , L2 =      256 * 1024
                , L3 = 2 * 1024 * 1024;

    // mar"Буква" = macro kernel bandwidth 
    // e.g. marK = bandwidth of L1 = 512 bit
    constexpr int marK = std::min(L1 / 4 / 16,   K) /  4 *  4;
    constexpr int marM = std::min(L2 / 4 / marK, M) /  6 *  6;
    constexpr int marN = std::min(L3 / 4 / marK, N) / 16 * 16;

    Buf bufB(marN * marK);
    Buf bufA(marK * marM);

    // chunk of B (chunk(B)) is stored in L3
    for(std::size_t j = 0u; j < N; j += marN)
    {
        std::size_t const dN = std::min(N, j + marN) - j;

        // chunk(chunk(B)) is stored in L1
        for(std::size_t k = 0u; k < K; k += marK)
        {
            std::size_t const dK = std::min(K, k + marK) - k;

            // chunk(A) is stored in L2
            for(std::size_t i = 0u; i < M; i += marM)
            {
                std::size_t const dM = std::min(M, i + marM) - i;

                reorderA<ABI<4u>>(A.memory() + i * K + k, K, dM, dK, bufA.p);
                macro   <ABI<8u>>
                (
                    dM, dN, dK,
                    bufA.p, 
                    B.memory() + k * N + j, N, bufB.p, (i == 0u), 
                    C.memory() + i * N + j, N
                );
                /*
                 * pool.addTaskSeeFuture(macro<ABI<8u>>(...))
                 */
            }
        }
    }
    
    return C;
}


