#pragma once

#include <cstddef>

namespace memory {

void* allocate_aligned(size_t size);

template<typename T>
T* allocate_aligned(size_t count) {
	return reinterpret_cast<T*>(allocate_aligned(count * sizeof(T)));
}

void free_aligned(void* pointer);

}
