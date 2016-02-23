#pragma once

#include "light_material.hpp"

namespace scene { namespace material { namespace light {

class Sample;

class Constant : public Material {
public:

	Constant(Generic_sample_cache<Sample>& cache, std::shared_ptr<image::texture::Texture_2D> mask, bool two_sided,
			 const math::vec3& emission);

	virtual const material::Sample& sample(const shape::Differential& dg, const math::vec3& wo,
										   float time, float ior_i,
										   const image::texture::sampler::Sampler_2D& sampler,
										   uint32_t worker_id) final override;

	virtual math::vec3 sample_emission(math::float2 uv, float time,
										 const image::texture::sampler::Sampler_2D& sampler) const final override;

	virtual math::vec3 average_emission() const final override;

	virtual bool has_emission_map() const final override;

private:

	math::vec3 emission_;
};

}}}
