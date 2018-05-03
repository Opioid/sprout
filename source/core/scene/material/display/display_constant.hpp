#pragma once

#include "scene/material/material.hpp"

namespace scene::material::display {

class Constant : public Material {

public:

	Constant(const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(f_float3 wo, const Renderstate& rs,
										   Sampler_filter filter, sampler::Sampler& sampler,
										   const Worker& worker) const override final;

	virtual float3 sample_radiance(f_float3 wi, float2 uv, float area, float time,
								   Sampler_filter filter,
								   const Worker& worker) const override final;

	virtual float3 average_radiance(float area) const override final;

	virtual float ior() const override final;

	virtual size_t num_bytes() const override final;

	void set_emission(float3 const& radiance);
	void set_roughness(float roughness);
	void set_ior(float ior);

	static size_t sample_size();

private:

	Texture_adapter emission_map_;

	float3 emission_;

	float roughness_;

	float ior_;

	float f0_;

};

}
