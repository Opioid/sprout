#pragma once

#include "substitute_base_material.hpp"

namespace scene { namespace material { namespace substitute {

class Material_translucent : public Material_base {

public:

	Material_translucent(Sample_cache& sample_cache, const Sampler_settings& sampler_settings);

	virtual const material::Sample& sample(float3_p wo, const Renderstate& rs,
										   const Worker& worker,
										   Sampler_filter filter) final override;

	virtual float3 thin_absorption(float3_p wo, float3_p n, float2 uv,
								   float time, const Worker& worker,
								   Sampler_filter filter) const final override;

	virtual bool is_translucent() const final override;

	virtual size_t num_bytes() const final override;

	void set_thickness(float thickness);
	void set_attenuation_distance(float attenuation_distance);

private:

	float thickness_;
	float attenuation_distance_;
};

}}}
