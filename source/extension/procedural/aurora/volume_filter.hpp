#ifndef SU_EXTENSION_PROCEDURAL_AURORA_VOLUME_FILTER_HPP
#define SU_EXTENSION_PROCEDURAL_AURORA_VOLUME_FILTER_HPP

#include "image/typed_image.hpp"

namespace thread { class Pool; }

namespace procedural::aurora {

class Volume_filter {

public:

	Volume_filter(const int3& dimensions, float radius, float alpha, uint32_t num_buckets);
	~Volume_filter();

	void filter(float3* target, thread::Pool& pool) const;

private:

	void filter_slices(uint32_t id, int32_t begin, int32_t end, float3* target) const;

	void filter_z(uint32_t id, int32_t begin, int32_t end, float3* target) const;

	int3 dimensions_;

	int32_t kernel_width_;

	struct K {
		int32_t o;
		float w;
	};

	K* kernel_;

	uint32_t num_buckets_;

	float3** scratch_;
};

}

#endif
