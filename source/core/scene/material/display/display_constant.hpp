#pragma once

#include "scene/material/material.hpp"

namespace scene { namespace material { namespace display {

class Constant : public Material {

public:

	Constant(Sample_cache& sample_cache, const Sampler_settings& sampler_settings,
			 bool two_sided);

	virtual const material::Sample& sample(float3_p wo, const Renderstate& rs,
										   const Worker& worker,
										   Sampler_filter filter) final override;

	virtual float3 sample_radiance(float3_p wi, float2 uv,
								   float area, float time, const Worker& worker,
								   Sampler_filter filter) const final override;

	virtual float3 average_radiance(float area) const final override;

	virtual size_t num_bytes() const final override;

	void set_emission(float3_p radiance);
	void set_roughness(float roughness);
	void set_ior(float ior);

private:

	Texture_adapter emission_map_;

	float3 emission_;

	float roughness_;

	float f0_;
};

}}}
