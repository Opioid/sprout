#pragma once

#include <cstdint>

namespace scene { namespace material {

template<typename T>
class Generic_sample_cache {
public:

	Generic_sample_cache(uint32_t num_bins);
	~Generic_sample_cache();

	T& get(uint32_t id);

private:

	T* samples_;
};

}}
