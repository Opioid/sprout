#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"
#include "scene/material/coating/coating.hpp"

namespace scene { namespace material { namespace metallic_paint {

class Sample;

class Material : public material::Typed_material<Sample_cache<Sample>> {

public:

	using Sampler_filter = material::Sampler_settings::Filter;

	Material(Sample_cache<Sample>& cache,
			 const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(float3_p wo, const Renderstate& rs,
										   const Worker& worker,
										   Sampler_filter filter) final override;

	virtual size_t num_bytes() const final override;

	void set_color(float3_p a, float3_p b);
	void set_roughness(float roughness);

	void set_flakes_mask(const Texture_adapter& mask);
	void set_flakes_normal_map(const Texture_adapter& normal_map);
	void set_flakes_ior(float3_p ior);
	void set_flakes_absorption(float3_p absorption);
	void set_flakes_roughness(float roughness);

	void set_coating_weight(float weight);
	void set_coating_color(float3_p color);

	void set_clearcoat(float ior, float roughness);

protected:

	Texture_adapter flakes_mask_;
	Texture_adapter flakes_normal_map_;

	float3 color_a_;
	float3 color_b_;

	float a2_;

	float3 flakes_ior_;
	float3 flakes_absorption_;
	float  flakes_a2_;

	coating::Clearcoat coating_;
};

}}}

