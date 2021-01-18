#ifndef SU_BASE_MEMORY_ALIGN_HPP
#define SU_BASE_MEMORY_ALIGN_HPP

#include <cstddef>

#ifdef _WIN32
#include <malloc.h>
#else
#include <cstdlib>
#endif

namespace memory {

inline size_t constexpr L1_cache_line_size = 64;

template <typename T>
T* allocate_aligned(size_t count) {
#ifdef _WIN32
    return reinterpret_cast<T*>(_aligned_malloc(count * sizeof(T), L1_cache_line_size));
#else
    return reinterpret_cast<T*>(std::aligned_alloc(L1_cache_line_size, count * sizeof(T)));
#endif
}

static inline void free_aligned(void* pointer) {
#ifdef _WIN32
    _aligned_free(pointer);
#else
    std::free(pointer);
#endif
}

}  // namespace memory

#endif
