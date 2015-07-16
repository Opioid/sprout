#pragma once

#include "matte.hpp"

namespace scene { namespace material { namespace matte {

class Constant : public Matte {
public:

	Constant(Sample_cache<Sample>& cache, std::shared_ptr<image::texture::Texture_2D> mask, const math::float3& color, float sqrt_roughness);

	virtual const Sample& sample(const shape::Differential& dg, const math::float3& wo,
								 const image::texture::sampler::Sampler_2D& sampler, uint32_t worker_id);

private:

	math::float3 color_;
	float sqrt_roughness_;
};

}}}

