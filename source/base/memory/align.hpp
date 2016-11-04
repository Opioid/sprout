#pragma once

#include <cstddef>

namespace memory {

void* allocate_aligned(size_t size);

template<typename T>
T* allocate_aligned(size_t count);

void free_aligned(void* pointer);

}
