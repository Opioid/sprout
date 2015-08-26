#pragma once

#include "scene/material/material.hpp"
#include "substitute_sample_cache.hpp"

namespace scene { namespace material { namespace substitute {

class Base {
public:

	void set_color(const math::float3& color);
	void set_color(std::shared_ptr<image::texture::Texture_2D> color);

	void set_normal(std::shared_ptr<image::texture::Texture_2D> normal);

	void set_roughness(float roughness);
	void set_metallic(float metallic);
	void set_surface(std::shared_ptr<image::texture::Texture_2D> surface);

	void set_emission(std::shared_ptr<image::texture::Texture_2D> emission);

protected:

	std::shared_ptr<image::texture::Texture_2D> color_map_;
	std::shared_ptr<image::texture::Texture_2D> normal_map_;
	std::shared_ptr<image::texture::Texture_2D> surface_map_;
	std::shared_ptr<image::texture::Texture_2D> emission_map_;

	math::float3 color_;
	float roughness_;
	float metallic_;

	float emission_factor_;
};

template<bool Two_sided, bool Color_map, bool Normal_map, bool Surface_map, bool Emission_map>
class Substitute : public Material<Sample_cache>, public Base {
public:

	Substitute(Sample_cache& cache,
			   std::shared_ptr<image::texture::Texture_2D> mask);

	virtual const material::Sample& sample(const shape::Differential& dg, const math::float3& wo,
										   const image::texture::sampler::Sampler_2D& sampler,
										   uint32_t worker_id) final override;

	virtual math::float3 sample_emission(math::float2 uv,
										 const image::texture::sampler::Sampler_2D& sampler) const override;

	virtual math::float3 average_emission() const override;

	virtual const image::texture::Texture_2D* emission_map() const override;
};

}}}
