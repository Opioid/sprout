#pragma once

#include "postprocessor.hpp"

namespace rendering { namespace postprocessor {

class Glare : public Postprocessor {

public:

	enum class Adaption {
		Scotopic,
		Mesopic,
		Photopic
	};

	Glare(Adaption adaption, float threshold, float intensity);
	~Glare();

	virtual void init(const scene::camera::Camera& camera, thread::Pool& pool) override final;

	virtual size_t num_bytes() const override final;

private:

	virtual void apply(int32_t begin, int32_t end, uint32_t pass,
					   const image::Float_4& source,
					   image::Float_4& destination) override final;

	Adaption adaption_;
	float threshold_;
	float intensity_;

//	image::Float_3 high_pass_;
	int2 dimensions_;
	float3* high_pass_;

	int2 kernel_dimensions_;
	float3* kernel_;
};

}}
