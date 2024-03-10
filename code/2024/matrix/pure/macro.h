void macro( int M, int N, int K
          , const float * A
          , const float * B, int ldb, float * bufB, bool reorderB
          ,       float * C, int ldc
          )
{
    for (int j = 0; j < N; j += 16) // cycle 3: micro по reordered B (in L3),
                                    // optionally reorder rest of  B
    {
        if(reorderB)
            reorder_b_16(K, B + j, ldb,  bufB + K * j);

        for (int i = 0; i < M; i += 6) // cycle 2: micro по reordered A (in L2)
              micro_6x16
              (
                  K, A + i * K, 1, 6, 
                  bufB + K * j,  16, 
                  C + i * ldc + j, ldc
              );
    }
}
