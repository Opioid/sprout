#pragma once

#include "material_sample_cache.hpp"

namespace scene { namespace material {

template<typename T>
T& Sample_cache::get(uint32_t id) {
	return *new(&buffer_[bin_size_ * id]) T;
}

}}
