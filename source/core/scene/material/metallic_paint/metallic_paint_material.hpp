#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"

namespace scene { namespace material { namespace metallic_paint {

class Sample;

class Material : public material::Typed_material<Generic_sample_cache<Sample>> {

public:

	Material(Generic_sample_cache<Sample>& cache, Texture_2D_ptr mask,
			 const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(float3_p wo, const Renderstate& rs,
										   const Worker& worker,
										   Sampler_settings::Filter filter) final override;

	void set_normal_map(Texture_2D_ptr normal_map);

	void set_ior(float3_p ior);
	void set_absorption(float3_p absorption);
	void set_roughness(float roughness);

protected:

	Texture_2D_ptr normal_map_;

	float3 ior_;
	float3 absorption_;

	float roughness_;
};

}}}

