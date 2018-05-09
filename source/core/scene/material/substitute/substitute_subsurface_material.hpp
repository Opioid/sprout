#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_SUBSURFACE_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_SUBSURFACE_MATERIAL_HPP

#include "substitute_base_material.hpp"
#include "substitute_subsurface_sample.hpp"

namespace scene::material::substitute {

class Material_subsurface final : public Material_base {

public:

	Material_subsurface(Sampler_settings const& sampler_settings);

	virtual const material::Sample& sample(f_float3 wo, const Renderstate& rs,
										   Sampler_filter filter, sampler::Sampler& sampler,
										   Worker const& worker) const override final;

	virtual size_t num_bytes() const override final;

	void set_attenuation(float3 const& absorption_color, float3 const& scattering_color,
						 float distance);

	void set_volumetric_anisotropy(float anisotropy);

	virtual void set_ior(float ior, float external_ior = 1.f) final override;

	virtual float3 emission(math::Ray const& ray, Transformation const& transformation,
							float step_size, rnd::Generator& rng,
							Sampler_filter filter, Worker const& worker) const override final;

	virtual float3 absorption_coefficient(float2 uv, Sampler_filter filter,
										  Worker const& worker) const override final;

	virtual CE collision_coefficients(float2 uv, Sampler_filter filter,
									  Worker const& worker) const override final;

	virtual CE collision_coefficients(f_float3 p, Transformation const& transformation,
									  Sampler_filter filter,
									  Worker const& worker) const override final;

	virtual CE collision_coefficients(f_float3 p, Sampler_filter filter,
									  Worker const& worker) const override final;

	static size_t sample_size();

private:

	float3 absorption_color_;
	float3 absorption_coefficient_;
	float3 scattering_coefficient_;
	float anisotropy_;
	float attenuation_distance_;

	Sample_subsurface::IOR sior_;
};

}

#endif
