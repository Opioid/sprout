#pragma once

#include "postprocessor.hpp"

namespace rendering { namespace postprocessor {

class Glare : public Postprocessor {

public:

	Glare(float threshold, float intensity);

	virtual void init(const scene::camera::Camera& camera) final override;

	virtual size_t num_bytes() const final override;

private:

	virtual void apply(int32_t begin, int32_t end, uint32_t pass,
					   const image::Image_float_4& source,
					   image::Image_float_4& destination) final override;

	float threshold_;
	float intensity_;

	struct K {
		int2 p;
		float w;
	};

	std::vector<K> kernel_;
};

}}
