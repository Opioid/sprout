#pragma once

#include "scene/material/material.hpp"

namespace image { namespace texture { namespace sampler { class Sampler_2D; }}}

namespace scene { namespace material { namespace substitute {

class Material_base : public material::Material {

public:

	Material_base(Sample_cache& sample_cache, const Sampler_settings& sampler_settings,
				  bool two_sided);

	virtual float3 sample_radiance(float3_p wi, float2 uv, float area, float time,
								   const Worker& worker,
								   Sampler_filter filter) const final override;

	virtual float3 average_radiance(float area) const final override;

	virtual bool has_emission_map() const final override;

	void set_color_map(const Texture_adapter& color_map);
	void set_normal_map(const Texture_adapter& normal_map);
	void set_surface_map(const Texture_adapter& surface_map);
	void set_emission_map(const Texture_adapter& emission_map);

	void set_color(float3_p color);
	void set_ior(float ior, float external_ior = 1.f);
	void set_roughness(float roughness);
	void set_metallic(float metallic);
	void set_emission_factor(float emission_factor);

protected:

	using Texture_sampler_2D = image::texture::sampler::Sampler_2D;

	template<typename Sample>
	void set_sample(float3_p wo, const Renderstate& rs,
					const Texture_sampler_2D& sampler, Sample& sample);

	Texture_adapter color_map_;
	Texture_adapter normal_map_;
	Texture_adapter surface_map_;
	Texture_adapter emission_map_;

	float3 color_;
	float ior_;
	float constant_f0_;
	float roughness_;
	float metallic_;
	float emission_factor_;
};

}}}
