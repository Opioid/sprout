#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"

namespace scene { namespace material { namespace cloth {

class Sample;

class Material : public material::Typed_material<Generic_sample_cache<Sample>> {

public:

	Material(Generic_sample_cache<Sample>& cache,
			 const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(float3_p wo, const Renderstate& rs,
										   const Worker& worker,
										   Sampler_filter filter) final override;

	void set_color_map(Texture_2D_ptr color_map);
	void set_normal_map(Texture_2D_ptr normal_map);

	void set_color(float3_p color);

private:

	Texture_2D_ptr color_map_;
	Texture_2D_ptr normal_map_;

	float3 color_;
};

}}}
