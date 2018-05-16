#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_DENSITY_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_DENSITY_HPP

#include "volumetric_material.hpp"

namespace scene::material::volumetric {

class Density : public Material {

public:

	Density(Sampler_settings const& sampler_settings);

	virtual float3 emission(math::Ray const& ray,Transformation const& transformation,
							float step_size, rnd::Generator& rng,
							Sampler_filter filter, Worker const& worker) const override final;

	virtual CE collision_coefficients(float2 uv, Sampler_filter filter,
									  Worker const& worker) const override final;

	virtual CE collision_coefficients(f_float3 p, Sampler_filter filter,
									  Worker const& worker) const override final;

private:

	// expects p in object space!
	virtual float density(f_float3 p, Sampler_filter filter, Worker const& worker) const = 0;
};

}

#endif
