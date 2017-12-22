#pragma once

#include "scene/material/material.hpp"

namespace scene::material::glass {

class Glass_rough : public Material {

public:

	Glass_rough(const Sampler_settings& sampler_settings);

	virtual const material::Sample& sample(const float3& wo, const Renderstate& rs,
										   Sampler_filter filter,
										   const Worker& worker) const override final;

	virtual size_t num_bytes() const override final;

	void set_normal_map(const Texture_adapter& normal_map);
	void set_roughness_map(const Texture_adapter& roughness_map);

	void set_refraction_color(const float3& color);
	void set_absorption_color(const float3& color);
	void set_attenuation_distance(float attenuation_distance);
	void set_ior(float ior);
	void set_roughness(float roughness);

protected:

	Texture_adapter normal_map_;
	Texture_adapter roughness_map_;

	float3 refraction_color_;
	float3 absorption_color_;
	float attenuation_distance_;
	float ior_;
	float alpha_;
};

}

