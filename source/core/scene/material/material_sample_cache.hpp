#pragma once

#include "bssrdf.hpp"
#include <cstddef>
#include <cstdint>

namespace scene { namespace material {

class BSSRDF;

class Sample_cache {

public:

	Sample_cache();
	~Sample_cache();

	void init(uint32_t max_sample_size);

	template<typename T>
	T& get();

	BSSRDF& bssrdf();

	size_t num_bytes() const;

private:

	uint32_t buffer_size_;
	char* buffer_;

	BSSRDF bssrdf_;
};

}}
