#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"

namespace image { namespace texture { namespace sampler { class Sampler_2D; }}}

namespace scene { namespace material { namespace substitute {

template<typename Sample>
class Material_base : public material::Typed_material<Generic_sample_cache<Sample>> {

public:

	using Sampler_filter = material::Sampler_settings::Filter;

	Material_base(Generic_sample_cache<Sample>& cache,
				  std::shared_ptr<image::texture::Texture_2D> mask,
				  const Sampler_settings& sampler_settings, bool two_sided);

	virtual float3 sample_radiance(float3_p wi, float2 uv,
										 float area, float time, const Worker& worker,
										 Sampler_filter filter) const final override;

	virtual float3 average_radiance(float area) const final override;

	virtual bool has_emission_map() const final override;

	void set_color_map(std::shared_ptr<image::texture::Texture_2D> color_map);
	void set_normal_map(std::shared_ptr<image::texture::Texture_2D> normal_map);
	void set_surface_map(std::shared_ptr<image::texture::Texture_2D> surface_map);
	void set_emission_map(std::shared_ptr<image::texture::Texture_2D> emission_map);

	void set_color(float3_p color);
	void set_ior(float ior, float external_ior = 1.f);
	void set_roughness(float roughness);
	void set_metallic(float metallic);
	void set_emission_factor(float emission_factor);

protected:

	using Texture_sampler_2D = image::texture::sampler::Sampler_2D;

	void set_sample(const shape::Hitpoint& hp, float3_p wo,
					const Texture_sampler_2D& sampler, Sample& sample);

	std::shared_ptr<image::texture::Texture_2D> color_map_;
	std::shared_ptr<image::texture::Texture_2D> normal_map_;
	std::shared_ptr<image::texture::Texture_2D> surface_map_;
	std::shared_ptr<image::texture::Texture_2D> emission_map_;

	float3 color_;
	float ior_;
	float constant_f0_;
	float a2_;
	float metallic_;
	float emission_factor_;
};

}}}
