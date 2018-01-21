#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_SUBSURFACE_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_SUBSURFACE_MATERIAL_HPP

#include "substitute_base_material.hpp"
#include "substitute_subsurface_sample.hpp"

namespace scene::material::substitute {

class Material_subsurface final : public Material_base {

public:

	Material_subsurface(const Sampler_settings& sampler_settings);

	virtual const material::Sample& sample(const float3& wo, const Renderstate& rs,
										   Sampler_filter filter, sampler::Sampler& sampler,
										   const Worker& worker) const override final;

	virtual size_t num_bytes() const override final;

	void set_attenuation(const float3& absorption_color, const float3& scattering_color,
						 float distance);

	void set_volumetric_anisotropy(float anisotropy);

	virtual void set_ior(float ior, float external_ior = 1.f) final override;

	static size_t sample_size();

private:

	float3 absorption_color_;
	float3 absorption_coefficient_;
	float3 scattering_coefficient_;
	float anisotropy_;
	float attenuation_distance_;

	Sample_subsurface::IOR ior_;
};

}

#endif
