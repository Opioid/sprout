#include "align.hpp"

#ifdef _WIN32
#	include <malloc.h>
#else
#	include <cstdlib>
#endif

namespace memory {

constexpr size_t L1_cache_line_size = 64;

void* allocate_aligned(size_t size) {
#ifdef _WIN32

	return _aligned_malloc(size, L1_cache_line_size);

#else

	void* pointer;
	posix_memalign(&pointer, L1_cache_line_size, size);
	return pointer;

#endif
}

void free_aligned(void* pointer) {
#ifdef _WIN32

	_aligned_free(pointer);

#else

	free(pointer);

#endif
}

}
