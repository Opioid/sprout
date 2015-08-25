#pragma once

#include "material_sample_cache.hpp"

namespace scene { namespace material {

template<typename T>
Generic_sample_cache<T>::Generic_sample_cache(uint32_t num_bins) : samples_(new T[num_bins]) {}

template<typename T>
Generic_sample_cache<T>::~Generic_sample_cache() {
	delete [] samples_;
}

template<typename T>
T& Generic_sample_cache<T>::get(uint32_t id) {
	return samples_[id];
}

}}
