#pragma once

#include "scene/material/material.hpp"
#include "image/texture/texture.hpp"

namespace scene { namespace material { namespace glass {

class Glass_rough : public Material {

public:

	Glass_rough(Sample_cache& sample_cache, const Sampler_settings& sampler_settings);

	virtual const material::Sample& sample(float3_p wo, const Renderstate& rs,
										   const Worker& worker,
										   Sampler_filter filter) final override;

	virtual size_t num_bytes() const final override;

	void set_normal_map(const Texture_adapter& normal_map);
	void set_roughness_map(const Texture_adapter& roughness_map);

	void set_refraction_color(float3_p color);
	void set_absorption_color(float3_p color);
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
	float a2_;
};

}}}

