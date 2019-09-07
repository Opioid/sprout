#include "align.hpp"

#ifdef _WIN32
#include <malloc.h>
#else
#include <cstdlib>
#endif

namespace memory {

void* allocate_aligned(size_t size) noexcept {
#ifdef _WIN32
    return _aligned_malloc(size, L1_cache_line_size);
#else
    void* pointer;
    if (0 == posix_memalign(&pointer, L1_cache_line_size, size)) {
        return pointer;
    }

    return nullptr;
#endif
}

void free_aligned(void* pointer) noexcept {
#ifdef _WIN32
    _aligned_free(pointer);
#else
    free(pointer);
#endif
}

}  // namespace memory
