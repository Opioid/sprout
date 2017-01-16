#pragma once

#include "scene/material/material.hpp"
#include "image/texture/texture.hpp"

namespace scene { namespace material { namespace glass {

class Thinglass : public Material {

public:

	Thinglass(Sample_cache& sample_cache, const Sampler_settings& sampler_settings);

	virtual const material::Sample& sample(float3_p wo, const Renderstate& rs,
										   const Worker& worker,
										   Sampler_filter filter) final override;

	virtual float3 thin_absorption(float3_p wo, float3_p n, float2 uv,
								   float time, const Worker& worker,
								   Sampler_filter filter) const final override;

	virtual bool is_translucent() const final override;

	virtual size_t num_bytes() const final override;

	void set_normal_map(const Texture_adapter& normal_map);

	void set_refraction_color(float3_p color);
	void set_absorbtion_color(float3_p color);
	void set_attenuation_distance(float attenuation_distance);
	void set_ior(float ior);
	void set_thickness(float thickness);

private:

	Texture_adapter normal_map_;

	float3 refraction_color_;
	float3 absorbtion_color_;
	float attenuation_distance_;
	float ior_;
	float thickness_;
};

}}}
