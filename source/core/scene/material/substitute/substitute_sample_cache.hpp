#pragma once

#include "substitute_sample.hpp"

namespace scene { namespace material { namespace substitute {

class Sample_cache {
public:

	Sample_cache(uint32_t num_bins);
	~Sample_cache();

	template<bool Thin>
	Sample<Thin>& get(uint32_t id);

private:

	Sample<false>* samples_;
	Sample<true>*  thin_samples_;
};

}}}
