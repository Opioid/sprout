#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"

namespace scene { namespace material { namespace cloth {

class Sample;

class Material : public material::Material<Generic_sample_cache<Sample>> {
public:

	Material(Generic_sample_cache<Sample>& cache, std::shared_ptr<image::texture::Texture_2D> mask, bool two_sided);

	virtual const material::Sample& sample(const shape::Differential& dg, const math::float3& wo, float ior_i,
										   const image::texture::sampler::Sampler_2D& sampler,
										   uint32_t worker_id) final override;

	virtual math::float3 sample_emission(math::float2 uv,
										 const image::texture::sampler::Sampler_2D& sampler) const final override;

	virtual math::float3 average_emission() const final override;

	virtual const image::texture::Texture_2D* emission_map() const final override;

	void set_color_map(std::shared_ptr<image::texture::Texture_2D> color_map);
	void set_normal_map(std::shared_ptr<image::texture::Texture_2D> normal_map);

	void set_color(const math::float3& color);

private:

	std::shared_ptr<image::texture::Texture_2D> color_map_;
	std::shared_ptr<image::texture::Texture_2D> normal_map_;

	math::float3 color_;
};

}}}
