#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_ATMOSPHERE_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_ATMOSPHERE_HPP

#include "volumetric_material.hpp"

namespace scene::material::volumetric {

class Atmosphere final : public Material {

public:

	Atmosphere(Sampler_settings const& sampler_settings);

	virtual float3 emission(Transformation const& transformation, math::Ray const& ray,
							float step_size, rnd::Generator& rng,
							Sampler_filter filter, Worker const& worker) const override final;

	virtual float3 optical_depth(Transformation const& transformation, math::AABB const& aabb,
								 math::Ray const& ray, float step_size, rnd::Generator& rng,
								 Sampler_filter filter, Worker const& worker) const override final;

	virtual float3 scattering(Transformation const& transformation, f_float3 p,
							  Sampler_filter filter, Worker const& worker) const override final;

	virtual size_t num_bytes() const override final;
};

}

#endif

