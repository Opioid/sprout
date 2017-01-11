#pragma once

#include <cstdint>

namespace scene { namespace material {

class BSSRDF;

template<typename T>
class Sample_cache {

public:

	Sample_cache(uint32_t num_bins);
	~Sample_cache();

	T& get(uint32_t id);

private:

	T* samples_;
};

class Sample_cache2 {

public:

	Sample_cache2(uint32_t bin_size, uint32_t num_bins);
	~Sample_cache2();

	template<typename T>
	T& get(uint32_t id);

	BSSRDF& bssrdf(uint32_t id);

private:

	uint32_t bin_size_;
	char* buffer_;

	BSSRDF* bssrdfs_;
};

}}
