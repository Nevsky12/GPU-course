#include "defs.h"
#include "micro.h"
#include "reorder.h"
#include "macro.h"
#include "init.h"

void gemm( int M, int N, int K
         , const float * A
         , const float * B
         ,       float * C
         )
{
    const int L1 =       32 * 1024
            , L2 =      256 * 1024
            , L3 = 2 * 1024 * 1024;

    int mK = std::min(L1 / 4 / 16, K) /  4 *  4;
    int mM = std::min(L2 / 4 / mK, M) /  6 *  6;
    int mN = std::min(L3 / 4 / mK, N) / 16 * 16;

    buf_t bufB(mN * mK);
    buf_t bufA(mK * mM);

    for (int j = 0; j < N; j += mN) // cycle 6: macro for B
    {
        int dN = std::min(N, j + mN) - j;


        for (int k = 0; k < K; k += mK) // cycle 5: macro for A и B
        {
            int dK = std::min(K, k + mK) - k;


            for (int i = 0; i < M; i += mM) // cycle 4: macro for A and reorder A,
                                            //          optionally init C 
            {
                int dM = std::min(M, i + mM) - i;


                if (k == 0)
                    init_c(dM, dN, C + i * N + j, N);


                reorder_a_6(A + i * K + k, K, dM, dK, bufA.p);
                macro
                ( 
                    dM, dN, dK, 
                    bufA.p, 
                    B + k * N + j, N, bufB.p, i == 0, 
                    C + i * N + j, N
                );
            }
        }
    }
}
