#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"

namespace scene { namespace material { namespace substitute {

class Sample;

class Material : public material::Material<Generic_sample_cache<Sample>> {
public:

	Material(Generic_sample_cache<Sample>& cache, std::shared_ptr<image::texture::Texture_2D> mask, bool two_sided);

	virtual const material::Sample& sample(const shape::Differential& dg, const math::float3& wo,
										   float time, float ior_i,
										   const image::texture::sampler::Sampler_2D& sampler,
										   uint32_t worker_id) final override;

	virtual math::float3 sample_emission(math::float2 uv, float time,
										 const image::texture::sampler::Sampler_2D& sampler) const final override;

	virtual math::float3 average_emission() const final override;

	virtual bool has_emission_map() const final override;

	void set_color_map(std::shared_ptr<image::texture::Texture_2D> color_map);
	void set_normal_map(std::shared_ptr<image::texture::Texture_2D> normal_map);
	void set_surface_map(std::shared_ptr<image::texture::Texture_2D> surface_map);
	void set_emission_map(std::shared_ptr<image::texture::Texture_2D> emission_map);

	void set_color(const math::float3& color);
	void set_ior(float ior);
	void set_roughness(float roughness);
	void set_metallic(float metallic);
	void set_emission_factor(float emission_factor);
	void set_thickness(float thickness);
	void set_attenuation_distance(float attenuation_distance);

private:

	std::shared_ptr<image::texture::Texture_2D> color_map_;
	std::shared_ptr<image::texture::Texture_2D> normal_map_;
	std::shared_ptr<image::texture::Texture_2D> surface_map_;
	std::shared_ptr<image::texture::Texture_2D> emission_map_;

	math::float3 color_;
	float constant_f0_;
	float roughness_;
	float metallic_;
	float emission_factor_;
	float thickness_;
	float attenuation_distance_;
};

}}}
