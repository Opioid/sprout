#ifndef SU_CORE_SCENE_MATERIAL_SAMPLE_CACHE_HPP
#define SU_CORE_SCENE_MATERIAL_SAMPLE_CACHE_HPP

#include "bssrdf.hpp"
#include <cstddef>
#include <cstdint>

namespace scene::material {

class BSSRDF;

class Sample_cache {

public:

	~Sample_cache();

	void init(uint32_t max_sample_size);

	template<typename T>
	T& get();

	BSSRDF& bssrdf();

	size_t num_bytes() const;

private:

	uint32_t buffer_size_ = 0;
	char* buffer_ = nullptr;

	BSSRDF bssrdf_;
};

}

#endif
