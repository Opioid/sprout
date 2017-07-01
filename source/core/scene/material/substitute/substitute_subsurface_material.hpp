#pragma once

#include "substitute_base_material.hpp"

namespace scene { namespace material { namespace substitute {

class Material_subsurface : public Material_base {

public:

	Material_subsurface(const Sampler_settings& sampler_settings);

	virtual const material::Sample& sample(const float3& wo, const Renderstate& rs,
										   Worker& worker, Sampler_filter filter) final override;

	virtual size_t num_bytes() const final override;

	void set_absorption_color(const float3& color);
	void set_scattering_color(const float3& color);
	void set_attenuation_distance(float attenuation_distance);

private:

	float3 absorption_color_;
	float3 scattering_color_;
	float attenuation_distance_;
};

}}}

