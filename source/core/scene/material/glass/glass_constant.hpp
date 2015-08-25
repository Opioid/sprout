#pragma once

#include "glass.hpp"

namespace scene { namespace material { namespace glass {

class Constant : public Glass {
public:

	Constant(Generic_sample_cache<Sample>& cache, std::shared_ptr<image::texture::Texture_2D> mask,
			 const math::float3& color, float attenuation_distance, float ior);

	virtual const Sample& sample(const shape::Differential& dg, const math::float3& wo,
								 const image::texture::sampler::Sampler_2D& sampler,
								 uint32_t worker_id) final override;

private:

	math::float3 color_;
	math::float3 attenuation_;
	float ior_;
};

}}}
