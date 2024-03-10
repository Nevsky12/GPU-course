template<std::size_t Abi>
void macro( std::size_t const M
          , std::size_t const N
          , std::size_t const K
          , const float * A
          , const float * B, std::size_t LDB, float * bufB, bool notReordered
          ,       float * C, std::size_t LDC
       // , ThreadPool &pool   
          ) noexcept
{
    for(std::size_t j = 0; j < N; j += 16)
    {
        if(notReordered)
           reorderB<Abi>(K, B + j, LDB, bufB + K * j);

        for(std::size_t i = 0; i < M; i += 6)
            micro<Abi>
            (
                K, A + i * K, 1, 6, 
                bufB + K * j,  16, 
                C + i * LDC + j, LDC
            );
        /*
         *  pool.addTaskSeeFuture
         *  (
         *      micro<Abi>(...)
         *  )
         */
    }
}
