#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::volumetric {

class Material : public material::Material {

public:

	Material(const Sampler_settings& sampler_settings);
	virtual ~Material();

	virtual const material::Sample& sample(const float3& wo, const Renderstate& rs,
										   Sampler_filter filter,
										   const Worker& worker) override final;

	virtual bool is_volumetric() const override final;

	void set_attenuation(const float3& absorption_color, const float3& scattering_color,
						 float distance);

	void set_anisotropy(float anisotropy);

protected:

	static float phase_schlick(const float3& w, const float3& wp, float k);

	float3 absorption_coefficient_;
	float3 scattering_coefficient_;
	float anisotropy_;
};

}

#endif
