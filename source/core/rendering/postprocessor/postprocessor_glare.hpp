#pragma once

#include "postprocessor.hpp"
#include <vector>

namespace rendering { namespace postprocessor {

class Glare : public Postprocessor {

public:

	enum class Adaption {
		Scotopic,
		Mesopic,
		Photopic
	};

	Glare(Adaption adaption, float threshold, float intensity);

	virtual void init(const scene::camera::Camera& camera, thread::Pool& pool) final override;

	virtual size_t num_bytes() const final override;

private:

	virtual void apply(int32_t begin, int32_t end, uint32_t pass,
					   const image::Float_4& source,
					   image::Float_4& destination) final override;

	Adaption adaption_;
	float threshold_;
	float intensity_;

//	image::Float_3 high_pass_;
	std::vector<float3> high_pass_;

	std::vector<float3> kernel_;

	int2 kernel_dimensions_;
};

}}
