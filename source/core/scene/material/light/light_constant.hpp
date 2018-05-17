#pragma once

#include "scene/material/material.hpp"
#include "scene/light/emittance.hpp"

namespace scene::material::light {

class Constant : public Material {

public:

	Constant(Sampler_settings const& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(f_float3 wo, Renderstate const& rs,
										   Sampler_filter filter, sampler::Sampler& sampler,
										   Worker const& worker) const override final;

	virtual float3 sample_radiance(f_float3 wi, float2 uv, float area, float time,
								   Sampler_filter filter,
								   Worker const& worker) const override final;

	virtual float3 average_radiance(float area) const override final;

	virtual float ior() const override final;

	virtual bool has_emission_map() const override final;

	virtual size_t num_bytes() const override final;

	::light::Emittance& emittance();

	static size_t sample_size();

private:

	::light::Emittance emittance_;
};

}
