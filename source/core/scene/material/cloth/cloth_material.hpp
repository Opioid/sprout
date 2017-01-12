#pragma once

#include "scene/material/material.hpp"

namespace scene { namespace material { namespace cloth {

class Material : public material::Material {

public:

	Material(Sample_cache& sample_cache, const Sampler_settings& sampler_settings,
			 bool two_sided);

	virtual const material::Sample& sample(float3_p wo, const Renderstate& rs,
										   const Worker& worker,
										   Sampler_filter filter) final override;

	virtual size_t num_bytes() const final override;

	void set_color_map(const Texture_adapter& color_map);
	void set_normal_map(const Texture_adapter& normal_map);

	void set_color(float3_p color);

private:

	Texture_adapter color_map_;
	Texture_adapter normal_map_;

	float3 color_;
};

}}}
