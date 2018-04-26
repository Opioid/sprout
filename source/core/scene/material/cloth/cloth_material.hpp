#pragma once

#include "scene/material/material.hpp"

namespace scene::material::cloth {

class Material : public material::Material {

public:

	Material(const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(f_float3 wo, const Renderstate& rs,
										   Sampler_filter filter, sampler::Sampler& sampler,
										   const Worker& worker) const override final;

	virtual float ior() const override final;

	virtual size_t num_bytes() const override final;

	void set_color_map(const Texture_adapter& color_map);
	void set_normal_map(const Texture_adapter& normal_map);

	void set_color(const float3& color);

	static size_t sample_size();

private:

	Texture_adapter color_map_;
	Texture_adapter normal_map_;

	float3 color_;
};

}
