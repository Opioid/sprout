#pragma once

#include "scene/material/material.hpp"
#include "scene/material/coating/coating.hpp"

namespace scene { namespace material { namespace metallic_paint {

class Material : public material::Material {

public:

	Material(const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(const float3& wo, const Renderstate& rs,
										   Worker& worker, Sampler_filter filter) override final;

	virtual size_t num_bytes() const override final;

	void set_color(const float3& a, const float3& b);
	void set_roughness(float roughness);

	void set_flakes_mask(const Texture_adapter& mask);
	void set_flakes_normal_map(const Texture_adapter& normal_map);
	void set_flakes_ior(const float3& ior);
	void set_flakes_absorption(const float3& absorption);
	void set_flakes_roughness(float roughness);

	void set_coating_weight(float weight);
	void set_coating_color(const float3& color);

	void set_clearcoat(float ior, float roughness);

protected:

	Texture_adapter flakes_mask_;
	Texture_adapter flakes_normal_map_;

	float3 color_a_;
	float3 color_b_;

	float alpha_;
	float alpha2_;

	float3 flakes_ior_;
	float3 flakes_absorption_;
	float  flakes_alpha_;
	float  flakes_alpha2_;

	coating::Clearcoat coating_;
};

}}}

