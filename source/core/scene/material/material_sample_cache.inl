#pragma once

#include "material_sample_cache.hpp"

namespace scene { namespace material {

template<typename T>
Sample_cache<T>::Sample_cache(uint32_t num_bins) : samples_(new T[num_bins]) {}

template<typename T>
Sample_cache<T>::~Sample_cache() {
	delete [] samples_;
}

template<typename T>
T& Sample_cache<T>::get(uint32_t id) {
	return samples_[id];
}

template<typename T>
T& Sample_cache2::get(uint32_t id) {
	return *reinterpret_cast<T*>(buffer_[bin_size_ * id]);
}

}}
