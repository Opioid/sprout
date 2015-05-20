#pragma once

#include "glass.hpp"
#include "image/texture/texture_2d.hpp"

namespace scene { namespace material { namespace glass {

class Normalmap : public Glass {
public:

	Normalmap(Sample_cache<Sample>& cache, std::shared_ptr<image::Image> mask,
			  const math::float3& color, float attenuation_distance, float ior, std::shared_ptr<image::Image> normal);

	virtual const Sample& sample(const shape::Differential& dg, const math::float3& wo,
								 const image::sampler::Sampler_2D& sampler, uint32_t worker_id) final override;

private:

	math::float3 color_;
	math::float3 attenuation_;
	float ior_;
	float f0_;
	image::Texture_2D normal_;
};

}}}
