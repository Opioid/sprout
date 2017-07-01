#pragma once

#include "scene/material/material.hpp"

namespace scene { namespace material { namespace display {

class Constant : public Material {

public:

	Constant(const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(const float3& wo, const Renderstate& rs, Worker& worker,
										   Sampler_filter filter) final override;

	virtual float3 sample_radiance(const float3& wi, float2 uv, float area, float time,
								   Worker& worker, Sampler_filter filter) const final override;

	virtual float3 average_radiance(float area) const final override;

	virtual size_t num_bytes() const final override;

	void set_emission(const float3& radiance);
	void set_roughness(float roughness);
	void set_ior(float ior);

private:

	Texture_adapter emission_map_;

	float3 emission_;

	float roughness_;

	float f0_;
};

}}}
