#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"
#include "image/texture/texture_2d.hpp"

namespace scene { namespace material { namespace glass {

class Sample_rough;

class Glass_rough : public Typed_material<Generic_sample_cache<Sample_rough>> {
public:

	Glass_rough(Generic_sample_cache<Sample_rough>& cache, std::shared_ptr<image::texture::Texture_2D> mask);

	virtual const material::Sample& sample(const shape::Differential& dg, const math::float3& wo,
										   float time, float ior_i,
										   const image::texture::sampler::Sampler_2D& sampler,
										   uint32_t worker_id) final override;

	virtual math::float3 sample_emission(math::float2 uv, float time,
										 const image::texture::sampler::Sampler_2D& sampler) const final override;

	virtual math::float3 average_emission() const final override;

	virtual bool has_emission_map() const final override;

	void set_normal_map(std::shared_ptr<image::texture::Texture_2D> normal_map);

	void set_color(const math::float3& color);
	void set_attenuation_distance(float attenuation_distance);
	void set_ior(float ior);
	void set_roughness(float roughness);

protected:

	std::shared_ptr<image::texture::Texture_2D> normal_map_;

	math::float3 color_;
	float attenuation_distance_;
	float ior_;
	float roughness_;
};

}}}
