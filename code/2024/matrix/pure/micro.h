void micro_6x16( int K, int step
               , const float * A, int lda
               , const float * B, int ldb, float * C, int ldc
               )
{
    __m256 c00 = _mm256_setzero_ps();
    __m256 c10 = _mm256_setzero_ps();
    __m256 c20 = _mm256_setzero_ps();
    __m256 c30 = _mm256_setzero_ps();
    __m256 c40 = _mm256_setzero_ps();
    __m256 c50 = _mm256_setzero_ps();
    __m256 c01 = _mm256_setzero_ps();
    __m256 c11 = _mm256_setzero_ps();
    __m256 c21 = _mm256_setzero_ps();
    __m256 c31 = _mm256_setzero_ps();
    __m256 c41 = _mm256_setzero_ps();
    __m256 c51 = _mm256_setzero_ps();

    const int offset0 = lda * 0;
    const int offset1 = lda * 1;
    const int offset2 = lda * 2;
    const int offset3 = lda * 3;
    const int offset4 = lda * 4;
    const int offset5 = lda * 5;

    __m256 b0, b1, a0, a1;

    for (int k = 0; k < K; k++) // cycle 1: reordered B is stored      in L1,
                                //          reordered A is stored      in L2,
                                //          sum         is accumulated in L0,
                                //          res         is recorded    in RAM;
                                //          
                                //          size of micro = 
                                //          length of SIMD vector
                                //                                      +
                                //          number of      vector regs;
                                //
                                //          length of cycle is defined by L1 size
                                //
    {
        b0  = _mm256_loadu_ps(B + 0);
        b1  = _mm256_loadu_ps(B + 8);
        a0  = _mm256_set1_ps(A[offset0]);
        a1  = _mm256_set1_ps(A[offset1]);

        c00 = _mm256_fmadd_ps(a0, b0, c00);
        c01 = _mm256_fmadd_ps(a0, b1, c01);
        c10 = _mm256_fmadd_ps(a1, b0, c10);
        c11 = _mm256_fmadd_ps(a1, b1, c11);
        
        //-----------------------------------//

        a0 = _mm256_set1_ps(A[offset2]);
        a1 = _mm256_set1_ps(A[offset3]);
        
        c20 = _mm256_fmadd_ps(a0, b0, c20);
        c21 = _mm256_fmadd_ps(a0, b1, c21);
        c30 = _mm256_fmadd_ps(a1, b0, c30);
        c31 = _mm256_fmadd_ps(a1, b1, c31);
        
        //-----------------------------------//

        a0 = _mm256_set1_ps(A[offset4]);
        a1 = _mm256_set1_ps(A[offset5]);
        
        c40 = _mm256_fmadd_ps(a0, b0, c40);
        c41 = _mm256_fmadd_ps(a0, b1, c41);
        c50 = _mm256_fmadd_ps(a1, b0, c50);
        c51 = _mm256_fmadd_ps(a1, b1, c51);
        
        //-----------------------------------//
        
        B += ldb; A += step;
    }
    _mm256_storeu_ps(C + 0, _mm256_add_ps(c00, _mm256_loadu_ps(C + 0)));
    _mm256_storeu_ps(C + 8, _mm256_add_ps(c01, _mm256_loadu_ps(C + 8)));
    C += ldc;

   //-------------------------------------------------------------------//

    _mm256_storeu_ps(C + 0, _mm256_add_ps(c10, _mm256_loadu_ps(C + 0)));
    _mm256_storeu_ps(C + 8, _mm256_add_ps(c11, _mm256_loadu_ps(C + 8)));
    C += ldc;

   //-------------------------------------------------------------------//
   
    _mm256_storeu_ps(C + 0, _mm256_add_ps(c20, _mm256_loadu_ps(C + 0)));
    _mm256_storeu_ps(C + 8, _mm256_add_ps(c21, _mm256_loadu_ps(C + 8)));
    C += ldc;

   //-------------------------------------------------------------------//

    _mm256_storeu_ps(C + 0, _mm256_add_ps(c30, _mm256_loadu_ps(C + 0)));
    _mm256_storeu_ps(C + 8, _mm256_add_ps(c31, _mm256_loadu_ps(C + 8)));
    C += ldc;

   //-------------------------------------------------------------------//

    _mm256_storeu_ps(C + 0, _mm256_add_ps(c40, _mm256_loadu_ps(C + 0)));
    _mm256_storeu_ps(C + 8, _mm256_add_ps(c41, _mm256_loadu_ps(C + 8)));
    C += ldc;

   //-------------------------------------------------------------------//

    _mm256_storeu_ps(C + 0, _mm256_add_ps(c50, _mm256_loadu_ps(C + 0)));
    _mm256_storeu_ps(C + 8, _mm256_add_ps(c51, _mm256_loadu_ps(C + 8)));
}
