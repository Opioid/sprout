#ifndef SU_BASE_MEMORY_ALIGN_HPP
#define SU_BASE_MEMORY_ALIGN_HPP

#include <cstddef>

namespace memory {

inline size_t constexpr L1_cache_line_size = 64;

void* allocate_aligned(size_t size);

template <typename T>
T* allocate_aligned(size_t count) {
    return reinterpret_cast<T*>(allocate_aligned(count * sizeof(T)));
}

void free_aligned(void* pointer);

template <class T>
void destroy(T* t) {
    if (t) {
        t->~T();
    }
}

template <class T>
void destroy(T& t) {
    t.~T();
}

template <typename T, typename... P>
T* construct_aligned(size_t count, P&... ps) {
    // This is more complicated than expected. See for example:
    // https://stackoverflow.com/questions/8720425/array-placement-new-requires-unspecified-overhead-in-the-buffer
    // Basically there is a small memory overhead for placement new[] that is "unknown" beforehand,
    // but needs to be allocated as well.

    //	return new(allocate_aligned<T>(count)) T[count];

    // It is easier to construct the array elements individually.

    T* buffer = allocate_aligned<T>(count);

    for (size_t i = 0; i < count; ++i) {
        new (&buffer[i]) T(ps...);
    }

    return buffer;
}

template <typename T>
void destroy_aligned(T* objects, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        objects[i].~T();
    }

    free_aligned(objects);
}

}  // namespace memory

#endif
