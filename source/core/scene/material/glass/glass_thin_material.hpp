#pragma once

#include "scene/material/material.hpp"

namespace scene::material::glass {

class Glass_thin : public Material {

public:

	Glass_thin(const Sampler_settings& sampler_settings);

	virtual const material::Sample& sample(const float3& wo, const Renderstate& rs,
										   Sampler_filter filter, sampler::Sampler& sampler,
										   const Worker& worker) const override final;

	virtual float3 thin_absorption(const float3& wo, const float3& n, float2 uv, float time,
								   Sampler_filter filter,
								   const Worker& worker) const override final;

	virtual float ior() const override final;

	virtual bool has_tinted_shadow() const override final;

	virtual bool is_scattering_volume() const final override;

	virtual size_t num_bytes() const override final;

	void set_normal_map(const Texture_adapter& normal_map);

	void set_refraction_color(const float3& color);
	void set_attenuation(const float3& absorption_color, float distance);
	void set_ior(float ior);
	void set_thickness(float thickness);

	static size_t sample_size();

private:

	Texture_adapter normal_map_;

	float3 refraction_color_;
	float3 absorption_coefficient_;
	float ior_;
	float thickness_;
};

}
