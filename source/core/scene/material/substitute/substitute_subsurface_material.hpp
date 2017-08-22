#pragma once

#include "substitute_base_material.hpp"
#include "substitute_subsurface_sample.hpp"

namespace scene { namespace material { namespace substitute {

class Material_subsurface : public Material_base {

public:

	Material_subsurface(const Sampler_settings& sampler_settings);

	virtual const material::Sample& sample(const float3& wo, const Renderstate& rs,
										   Worker& worker, Sampler_filter filter) override final;

	virtual size_t num_bytes() const override final;

	void set_absorption_color(const float3& color);
	void set_scattering_color(const float3& color);
	void set_attenuation_distance(float attenuation_distance);
	virtual void set_ior(float ior, float external_ior = 1.f) final override;

private:

	float3 absorption_color_;
	float3 scattering_color_;
	float attenuation_distance_;

	Sample_subsurface::IOR ior_;
};

}}}

