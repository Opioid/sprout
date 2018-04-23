#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_HOMOGENEOUS_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_HOMOGENEOUS_HPP

#include "volumetric_material.hpp"

namespace scene::material::volumetric {

class Homogeneous final : public Material {

public:

	Homogeneous(const Sampler_settings& sampler_settings);

	virtual float3 emission(const Transformation& transformation, const math::Ray& ray,
							float step_size, rnd::Generator& rng,
							Sampler_filter filter, const Worker& worker) const override final;

	virtual float3 absorption_coefficient(float2 uv, Sampler_filter filter,
										  const Worker& worker) const override final;

	virtual void collision_coefficients(float2 uv, Sampler_filter filter, const Worker& worker,
										float3& mu_a, float3& mu_s) const override final;

	virtual void collision_coefficients(const Transformation& transformation, const float3& p,
										Sampler_filter filter, const Worker& worker,
										float3& mu_a, float3& mu_s) const override final;

	virtual float majorant_mu_t() const override final;

	virtual size_t num_bytes() const override final;
};

}

#endif
