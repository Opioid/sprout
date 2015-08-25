#pragma once

#include "substitute.hpp"
#include "image/texture/texture_2d.hpp"

namespace scene { namespace material { namespace substitute {

template<bool Two_sided, bool Thin>
class Colormap_normalmap_surfacemap_emissionmap : public Substitute {
public:

	Colormap_normalmap_surfacemap_emissionmap(Sample_cache& cache,
											  std::shared_ptr<image::texture::Texture_2D> mask,
											  std::shared_ptr<image::texture::Texture_2D> color,
											  std::shared_ptr<image::texture::Texture_2D> normal,
											  std::shared_ptr<image::texture::Texture_2D> surface,
											  std::shared_ptr<image::texture::Texture_2D> emission,
											  float emission_factor);

	virtual const material::Sample& sample(const shape::Differential& dg, const math::float3& wo,
										   const image::texture::sampler::Sampler_2D& sampler,
										   uint32_t worker_id) final override;

	virtual math::float3 average_emission() const final override;

private:

	std::shared_ptr<image::texture::Texture_2D> color_;
	std::shared_ptr<image::texture::Texture_2D> normal_;
	std::shared_ptr<image::texture::Texture_2D> surface_;
	std::shared_ptr<image::texture::Texture_2D> emission_;
	float emission_factor_;
};

}}}
