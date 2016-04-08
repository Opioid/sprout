#pragma once

#include "light_material.hpp"

namespace scene { namespace material { namespace light {

class Sample;

class Constant : public Material {
public:

	Constant(Generic_sample_cache<Sample>& cache, std::shared_ptr<image::texture::Texture_2D> mask,
			 const Sampler_settings& sampler_settings, bool two_sided, const math::float3& emission);

	virtual const material::Sample& sample(const shape::Hitpoint& hp, math::pfloat3 wo,
										   float time, float ior_i,
										   const Worker& worker, Sampler_settings::Filter filter) final override;

	virtual math::float3 sample_emission(math::float2 uv, float time,
										 const Worker& worker, Sampler_settings::Filter filter) const final override;

	virtual math::float3 average_emission() const final override;

	virtual bool has_emission_map() const final override;

private:

	math::float3 emission_;
};

}}}
