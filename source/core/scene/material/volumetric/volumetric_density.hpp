#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_DENSITY_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_DENSITY_HPP

#include "volumetric_material.hpp"

namespace scene::material::volumetric {

class Density : public Material {

public:

	Density(const Sampler_settings& sampler_settings);

	virtual float3 emission(const Transformation& transformation, const math::Ray& ray,
							float step_size, rnd::Generator& rng,
							Sampler_filter filter, const Worker& worker) const override final;

	virtual float3 optical_depth(const Transformation& transformation, const math::AABB& aabb,
								 const math::Ray& ray, float step_size, rnd::Generator& rng,
								 Sampler_filter filter, const Worker& worker) const override;

	virtual float3 scattering(const Transformation& transformation, const float3& p,
							  Sampler_filter filter, const Worker& worker) const override final;

private:

	// expects p in object space!
	virtual float density(const Transformation& transformation, const float3& p,
						  Sampler_filter filter, const Worker& worker) const = 0;
};

}

#endif
