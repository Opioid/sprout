#pragma once

#include "substitute_sample_cache.hpp"

namespace scene { namespace material { namespace substitute {

Sample_cache::Sample_cache(uint32_t num_bins) :
	samples_(new Sample<false>[num_bins]),
	thin_samples_(new Sample<true>[num_bins]) {}

Sample_cache::~Sample_cache() {
	delete [] thin_samples_;
	delete [] samples_;
}

template<>
Sample<true>& Sample_cache::get<true>(uint32_t id) {
	return thin_samples_[id];
}

template<>
Sample<false>& Sample_cache::get<false>(uint32_t id) {
	return samples_[id];
}

}}}
