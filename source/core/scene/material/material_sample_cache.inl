#pragma once

#include "material_sample_cache.hpp"

namespace scene { namespace material {

template<typename T>
T& Sample_cache::get() {
	return *new(buffer_) T;
}

}}
