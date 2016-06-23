#pragma once

#include "light_material.hpp"
#include "scene/light/emittance.hpp"

namespace scene { namespace material { namespace light {

class Sample;

class Constant : public Material {

public:

	Constant(Generic_sample_cache<Sample>& cache,
			 std::shared_ptr<image::texture::Texture_2D> mask,
			 const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(const shape::Hitpoint& hp, float3_p wo,
										   float area, float time, float ior_i,
										   const Worker& worker,
										   Sampler_filter filter) final override;

	virtual float3 sample_radiance(float3_p wi, float2 uv,
										 float area, float time, const Worker& worker,
										 Sampler_filter filter) const final override;

	virtual float3 average_radiance(float area) const final override;

	virtual bool has_emission_map() const final override;

	::light::Emittance& emittance();

private:

	::light::Emittance emittance_;
};

}}}
