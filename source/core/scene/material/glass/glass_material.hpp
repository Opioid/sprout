#ifndef SU_CORE_SCENE_MATERIAL_GLASS_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::glass {

class Glass : public Material {

public:

	Glass(Sampler_settings const& sampler_settings);

	virtual const material::Sample& sample(f_float3 wo, Renderstate const& rs,
										   Sampler_filter filter, sampler::Sampler& sampler,
										   Worker const& worker) const override;

	virtual float3 absorption_coefficient(float2 uv, Sampler_filter filter,
							  Worker const& worker) const override final;

	virtual float ior() const override final;

	virtual bool is_scattering_volume() const override final;

	virtual size_t num_bytes() const override;

	void set_normal_map(Texture_adapter const& normal_map);

	void set_refraction_color(float3 const& color);
	void set_attenuation(float3 const& absorption_color, float distance);
	void set_ior(float ior);

	static size_t sample_size();

protected:

	Texture_adapter normal_map_;

	float3 refraction_color_;
	float3 absorption_coefficient_;
	float3 absorption_color_;
	float attenuation_distance_;
	float ior_;
};

}

#endif
