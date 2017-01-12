#pragma once

#include <cstdint>

namespace scene { namespace material {

class BSSRDF;

class Sample_cache {

public:

	Sample_cache(uint32_t bin_size, uint32_t num_bins);
	~Sample_cache();

	template<typename T>
	T& get(uint32_t id);

	BSSRDF& bssrdf(uint32_t id);

private:

	uint32_t bin_size_;
	char* buffer_;

	BSSRDF* bssrdfs_;
};

}}
