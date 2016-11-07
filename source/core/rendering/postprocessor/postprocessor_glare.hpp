#pragma once

#include "postprocessor.hpp"
#include <vector>

namespace rendering { namespace postprocessor {

class Glare : public Postprocessor {

public:

	Glare(float threshold, float intensity);

	virtual void init(const scene::camera::Camera& camera, thread::Pool& pool) final override;

	virtual size_t num_bytes() const final override;

private:

	struct Init {
		void init(uint32_t id);

		float  a_sum;
		float  b_sum;
		float  c_sum;
		float3 d_sum;
	};

	virtual void apply(int32_t begin, int32_t end, uint32_t pass,
					   const image::Image_float_4& source,
					   image::Image_float_4& destination) final override;

	float threshold_;
	float intensity_;

//	image::Image_float_3 high_pass_;
	std::vector<float3> high_pass_;

	std::vector<float3> kernel_;

	int2 kernel_dimensions_;
};

}}
