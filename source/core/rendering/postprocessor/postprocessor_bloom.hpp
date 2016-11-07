#pragma once

#include "postprocessor.hpp"
#include <vector>

namespace rendering { namespace postprocessor {

class Bloom : public Postprocessor {

public:

	Bloom(float angle, float alpha, float threshold, float intensity);

	virtual void init(const scene::camera::Camera& camera, thread::Pool& pool) final override;

	virtual size_t num_bytes() const final override;

private:

	virtual void apply(int32_t begin, int32_t end, uint32_t pass,
					   const image::Image_float_4& source,
					   image::Image_float_4& destination) final override;

	float angle_;
	float alpha_;
	float threshold_;
	float intensity_;

	image::Image_float_4 scratch_;

	struct K {
		int32_t o;
		float w;
	};

	std::vector<K> kernel_;
};

}}
