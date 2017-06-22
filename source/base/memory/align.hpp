#pragma once

#include <cstddef>

namespace memory {

void* allocate_aligned(size_t size);

template<typename T>
T* allocate_aligned(size_t count) {
	return reinterpret_cast<T*>(allocate_aligned(count * sizeof(T)));
}

void free_aligned(void* pointer);

template<class T>
void safe_destruct(T* t) {
	if (t) {
		t->~T();
	}
}

template<class T>
void safe_destruct(T& t) {
	t.~T();
}

template<typename T>
T* construct_aligned(size_t count) {
	// This is more complicated than expected. See for example:
	// https://stackoverflow.com/questions/8720425/array-placement-new-requires-unspecified-overhead-in-the-buffer
	// Basically there is a small memory overhead for placement new[] that is "unknown" beforehand,
	// but needs to be allocated as well

	//	return new(allocate_aligned<T>(count)) T[count];

	// It is easier to construct the array elements individually

	T* buffer = allocate_aligned<T>(count);

	for (size_t i = 0; i < count; ++i) {
		new(&buffer[i]) T;
	}

	return buffer;
}

template<typename T>
void destroy_aligned(T* objects, size_t count) {
	for (size_t i = 0; i < count; ++i) {
		objects[i].~T();
	}

	free_aligned(objects);
}

}
