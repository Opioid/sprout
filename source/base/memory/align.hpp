#pragma once

#include <cstddef>

namespace memory {

void* allocate_aligned(size_t size);

template<typename T>
T* allocate_aligned(size_t count) {
	return reinterpret_cast<T*>(allocate_aligned(count * sizeof(T)));
}

template<typename T>
T* construct_aligned(size_t count) {
	return new(allocate_aligned<T>(count)) T[count];
}

void free_aligned(void* pointer);

template<typename T>
void destroy_aligned(T* objects, size_t count) {
	for (size_t i = 0; i < count; ++i) {
		objects[i].~T();
	}

	free_aligned(objects);
}

}
