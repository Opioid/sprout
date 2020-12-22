#ifndef SU_BASE_MEMORY_ALIGN_HPP
#define SU_BASE_MEMORY_ALIGN_HPP

#include <cstddef>
#include <cstdlib>

namespace memory {

inline size_t constexpr L1_cache_line_size = 64;

template <typename T>
T* allocate_aligned(size_t count) {
    return reinterpret_cast<T*>(std::aligned_alloc(L1_cache_line_size, count * sizeof(T)));
}

}  // namespace memory

#endif
