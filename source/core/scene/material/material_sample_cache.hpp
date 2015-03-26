#pragma once

#include <cstdint>

namespace scene { namespace material {

template<typename T>
class Sample_cache {
public:

	Sample_cache(uint32_t num_bins);
	~Sample_cache();

	T& get(uint32_t id);

private:

	T* samples_;
};

}}
