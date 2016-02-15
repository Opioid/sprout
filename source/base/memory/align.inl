#pragma once

#include "align.hpp"

namespace memory {

template<typename T>
T* allocate_aligned(size_t count) {
	return reinterpret_cast<T*>(allocate_aligned(count * sizeof(T)));
}

}
