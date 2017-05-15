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

	virtual void pre_apply(const image::Float_4& source, image::Float_4& destination,
						   thread::Pool& pool) final override;

	virtual void apply(int32_t begin, int32_t end, uint32_t pass,
					   const image::Float_4& source,
					   image::Float_4& destination) final override;

	Adaption adaption_;
	float threshold_;
	float intensity_;

	int2 kernel_dimensions_;

	float2* kernel_dft_r_;
	float2* kernel_dft_g_;
	float2* kernel_dft_b_;

	float* high_pass_r_;
	float* high_pass_g_;
	float* high_pass_b_;

	float2* high_pass_dft_r_;
	float2* high_pass_dft_g_;
	float2* high_pass_dft_b_;
};

}}
