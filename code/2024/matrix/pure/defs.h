#pragma once
#include <immintrin.h>
#include <assert.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>

typedef void (*gemm_t)(int M, int N, int K, const float * A, const float * B, float * C);
        void   gemm   (int M, int N, int K, const float * A, const float * B, float * C);

void micro_6x16( int K, int step
               , const float * A, int lda
               , const float * B, int ldb
               ,       float * C, int ldc
               );

void init_c(int M, int N, float * C, int ldc);

void reorder_b_16(const float * B, int ldb,        int K, float *   _B);
void reorder_a_6 (const float * A, int lda, int M, int K, float * bufA);

struct buf_t
{
    float * p;
    int n;

    buf_t(int size) 
    : 
        n(size), 
        p((float*)_mm_malloc(size * 4, 64)) 
    {}

    ~buf_t() { _mm_free(p); }
};
