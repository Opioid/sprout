#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"

namespace scene { namespace material { namespace substitute {

template<typename Sample>
class Material_base : public material::Typed_material<Generic_sample_cache<Sample>> {

public:

	using Sampler_filter = material::Sampler_settings::Filter;

	Material_base(Generic_sample_cache<Sample>& cache,
				  std::shared_ptr<image::texture::Texture_2D> mask,
				  const Sampler_settings& sampler_settings, bool two_sided);

	virtual math::float3 sample_radiance(math::pfloat3 wi, math::float2 uv,
										 float area, float time, const Worker& worker,
										 Sampler_filter filter) const final override;

	virtual math::float3 average_radiance(float area) const final override;

	virtual bool has_emission_map() const final override;

	void set_color_map(std::shared_ptr<image::texture::Texture_2D> color_map);
	void set_normal_map(std::shared_ptr<image::texture::Texture_2D> normal_map);
	void set_surface_map(std::shared_ptr<image::texture::Texture_2D> surface_map);
	void set_emission_map(std::shared_ptr<image::texture::Texture_2D> emission_map);

	void set_color(math::pfloat3 color);
	void set_ior(float ior, float external_ior = 1.f);
	void set_roughness(float roughness);
	void set_metallic(float metallic);
	void set_emission_factor(float emission_factor);

protected:

	std::shared_ptr<image::texture::Texture_2D> color_map_;
	std::shared_ptr<image::texture::Texture_2D> normal_map_;
	std::shared_ptr<image::texture::Texture_2D> surface_map_;
	std::shared_ptr<image::texture::Texture_2D> emission_map_;

	math::float3 color_;
	float constant_f0_;
	float a2_;
	float metallic_;
	float emission_factor_;
};

}}}
