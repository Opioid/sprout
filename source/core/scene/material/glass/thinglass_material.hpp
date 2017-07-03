#pragma once

#include "scene/material/material.hpp"

namespace scene { namespace material { namespace glass {

class Thinglass : public Material {

public:

	Thinglass(const Sampler_settings& sampler_settings);

	virtual const material::Sample& sample(const float3& wo, const Renderstate& rs,
										   Worker& worker, Sampler_filter filter) override final;

	virtual float3 thin_absorption(const float3& wo, const float3& n, float2 uv, float time,
								   Worker& worker, Sampler_filter filter) const override final;

	virtual bool has_tinted_shadow() const override final;

	virtual size_t num_bytes() const override final;

	void set_normal_map(const Texture_adapter& normal_map);

	void set_refraction_color(const float3& color);
	void set_absorption_color(const float3& color);
	void set_attenuation_distance(float attenuation_distance);
	void set_ior(float ior);
	void set_thickness(float thickness);

private:

	Texture_adapter normal_map_;

	float3 refraction_color_;
	float3 absorption_color_;
	float attenuation_distance_;
	float ior_;
	float thickness_;
};

}}}
