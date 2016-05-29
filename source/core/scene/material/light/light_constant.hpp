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

	virtual const material::Sample& sample(const shape::Hitpoint& hp, math::pfloat3 wo,
										   float area, float time, float ior_i,
										   const Worker& worker,
										   Sampler_filter filter) final override;

	virtual math::float3 sample_radiance(math::pfloat3 wi, math::float2 uv,
										 float area, float time, const Worker& worker,
										 Sampler_filter filter) const final override;

	virtual math::float3 average_radiance() const final override;

	virtual bool has_emission_map() const final override;

	::light::Emittance& emittance();

	void set_emission(math::pfloat3 emission);

private:

	::light::Emittance emittance_;
};

}}}
