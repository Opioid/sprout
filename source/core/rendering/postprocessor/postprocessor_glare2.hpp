#pragma once

#include "postprocessor.hpp"

namespace rendering { namespace postprocessor {

class Glare2 : public Postprocessor {

public:

	enum class Adaption {
		Scotopic,
		Mesopic,
		Photopic
	};

	Glare2(Adaption adaption, float threshold, float intensity);
	~Glare2();

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
	int2 dimensions_;
	float3* high_pass_;

//	float*

	int2 kernel_dimensions_;
	float3* kernel_;
};

}}
