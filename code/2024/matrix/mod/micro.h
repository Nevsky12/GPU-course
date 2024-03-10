
template<std::size_t Abi>
void micro( std::size_t const K
          , std::size_t const step
          , const float * A, std::size_t LDA
          , const float * B, std::size_t LDB
          ,       float * C, std::size_t LDC
          ) noexcept
{
    vf32t<Abi> c00, c10, c20, c30, c40, c50,
               c01, c11, c21, c31, c41, c51;

    std::size_t const offset0 = LDA * 0u;
    std::size_t const offset1 = LDA * 1u;
    std::size_t const offset2 = LDA * 2u;
    std::size_t const offset3 = LDA * 3u;
    std::size_t const offset4 = LDA * 4u;
    std::size_t const offset5 = LDA * 5u;


    vf32t<Abi> a0, a1,
               b0, b1;

    for (std::size_t k = 0u; k < K; ++k)
    {
        b0.copy_from(B + 0u     , stdx::vector_aligned);
        b1.copy_from(B + 8u     , stdx::vector_aligned);
        a0.copy_from(&A[offset0], stdx::vector_aligned);
        a1.copy_from(&A[offset0], stdx::vector_aligned);

        c00 = a0 * b0 + c00;
        c01 = a0 * b1 + c01;
        c10 = a1 * b0 + c10;
        c11 = a1 * b1 + c11;
        
        a0.copy_from(&A[offset2], stdx::vector_aligned);
        a1.copy_from(&A[offset3], stdx::vector_aligned);
        
        c20 = a0 * b0 + c20;
        c21 = a0 * b1 + c21;
        c30 = a1 * b0 + c30;
        c31 = a1 * b1 + c31;
        
        a0.copy_from(&A[offset4], stdx::vector_aligned);
        a1.copy_from(&A[offset5], stdx::vector_aligned);
       
        c40 = a0 * b0 + c40;
        c41 = a0 * b1 + c41;
        c50 = a1 * b0 + c50;
        c51 = a1 * b1 + c51;
        
        B += LDB; A += step;
    }


    vf32t<Abi> tmp;


            tmp.copy_from(C + 0, stdx::vector_aligned);
    (c00 + tmp).copy_to  (C + 0, stdx::vector_aligned);

            tmp.copy_from(C + 8, stdx::vector_aligned);
    (c01 + tmp).copy_to  (C + 8, stdx::vector_aligned);

                          C += LDC;

            tmp.copy_from(C + 0, stdx::vector_aligned);
    (c10 + tmp).copy_to  (C + 0, stdx::vector_aligned);

            tmp.copy_from(C + 8, stdx::vector_aligned);
    (c11 + tmp).copy_to  (C + 8, stdx::vector_aligned);
    
                          C += LDC;

            tmp.copy_from(C + 0, stdx::vector_aligned);
    (c20 + tmp).copy_to  (C + 0, stdx::vector_aligned);

            tmp.copy_from(C + 8, stdx::vector_aligned);
    (c21 + tmp).copy_to  (C + 8, stdx::vector_aligned);
    
                          C += LDC;

            tmp.copy_from(C + 0, stdx::vector_aligned);
    (c30 + tmp).copy_to  (C + 0, stdx::vector_aligned);

            tmp.copy_from(C + 8, stdx::vector_aligned);
    (c31 + tmp).copy_to  (C + 8, stdx::vector_aligned);
    
                          C += LDC;

            tmp.copy_from(C + 0, stdx::vector_aligned);
    (c40 + tmp).copy_to  (C + 0, stdx::vector_aligned);

            tmp.copy_from(C + 8, stdx::vector_aligned);
    (c41 + tmp).copy_to  (C + 8, stdx::vector_aligned);
    
                          C += LDC;
   
            tmp.copy_from(C + 0, stdx::vector_aligned);
    (c50 + tmp).copy_to  (C + 0, stdx::vector_aligned);

            tmp.copy_from(C + 8, stdx::vector_aligned);
    (c51 + tmp).copy_to  (C + 8, stdx::vector_aligned);
}
