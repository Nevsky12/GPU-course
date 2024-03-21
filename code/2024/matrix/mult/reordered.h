#pragma once
#include "../tools/matrix.h"

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

