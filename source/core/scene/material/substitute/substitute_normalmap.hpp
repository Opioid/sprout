#pragma once

#include "substitute.hpp"
#include "image/texture/texture_2d.hpp"

namespace scene { namespace material { namespace substitute {

template<bool Two_sided, bool Thin>
class Normalmap : public Substitute {
public:

	Normalmap(Sample_cache& cache,
			  std::shared_ptr<image::texture::Texture_2D> mask,
			  const math::float3& color,
			  std::shared_ptr<image::texture::Texture_2D> normal,
			  float roughness,
			  float metallic);

	virtual const material::Sample& sample(const shape::Differential& dg, const math::float3& wo,
										   const image::texture::sampler::Sampler_2D& sampler,
										   uint32_t worker_id) final override;

private:

	math::float3 color_;
	std::shared_ptr<image::texture::Texture_2D> normal_;
	float roughness_;
	float metallic_;
};

}}}
