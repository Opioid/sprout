#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"

namespace scene { namespace material { namespace metallic_paint {

class Sample;

class Material : public material::Typed_material<Generic_sample_cache<Sample>> {

public:

	Material(Generic_sample_cache<Sample>& cache,
			 const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(float3_p wo, const Renderstate& rs,
										   const Worker& worker,
										   Sampler_settings::Filter filter) final override;

	void set_color(float3_p a, float3_p b);

protected:

	float3 color_a_;
	float3 color_b_;
};

}}}

