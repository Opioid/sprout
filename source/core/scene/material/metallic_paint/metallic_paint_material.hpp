#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"
#include "scene/material/coating/coating.hpp"

namespace scene { namespace material { namespace metallic_paint {

class Sample;

class Material : public material::Typed_material<Generic_sample_cache<Sample>> {

public:

	using Sampler_filter = material::Sampler_settings::Filter;

	Material(Generic_sample_cache<Sample>& cache,
			 const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(float3_p wo, const Renderstate& rs,
										   const Worker& worker,
										   Sampler_filter filter) final override;

	void set_flakes_mask(const Adapter_2D& mask);
	void set_flakes_normal_map(const Adapter_2D& normal_map);

	void set_color(float3_p a, float3_p b);

	void set_coating_weight(float weight);
	void set_coating_color(float3_p color);

	void set_clearcoat(float ior, float roughness);

protected:

	Adapter_2D flakes_mask_;
	Adapter_2D flakes_normal_map_;

	float3 color_a_;
	float3 color_b_;

	coating::Clearcoat coating_;
};

}}}

