#pragma once
#include <cassert>
#include <memory>
#include <algorithm>

template<typename T>
struct Matrix
{
    std::unique_ptr
    <
        T[],
        decltype([](void * const p) noexcept {std::free(p);})
    > memory;
    std::size_t memoryWidth, width, height;

    T       *operator[](std::size_t const rowI)       noexcept {assert(rowI < height); return memory.get() + rowI * memoryWidth;}
    T const *operator[](std::size_t const rowI) const noexcept {assert(rowI < height); return memory.get() + rowI * memoryWidth;}
};

template<typename T>
Matrix<T> emptyMatrix( std::size_t const width
                     , std::size_t const height
                     , std::size_t const align = std::max(sizeof(void *), alignof(T))
                     ) noexcept
{
    assert(align != 0u);
    std::size_t const rowbytes = ((sizeof(T) * width + align - 1) / align) * align;
    std::size_t const memoryWidth = rowbytes / sizeof(T);

    T * const memory = static_cast<T *>(std::aligned_alloc(align, rowbytes * height));
    for(std::size_t i = 0u; i < memoryWidth * height; ++i)
        memory[i] = T(0);

    return
    {
        .memory = {memory, {}},
        .memoryWidth = memoryWidth,
        .width = width,
        .height = height,
    };
}

