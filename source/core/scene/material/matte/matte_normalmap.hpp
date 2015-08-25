#pragma once

#include "matte.hpp"
#include "image/texture/texture_2d.hpp"

namespace scene { namespace material { namespace matte {

template<bool Two_sided>
class Normalmap : public Matte {
public:

	Normalmap(Generic_sample_cache<Sample>& cache,
			  std::shared_ptr<image::texture::Texture_2D> mask,
			  const math::float3& color,
			  std::shared_ptr<image::texture::Texture_2D> normal,
			  float sqrt_roughness);

	virtual const Sample& sample(const shape::Differential& dg, const math::float3& wo,
								 const image::texture::sampler::Sampler_2D& sampler,
								 uint32_t worker_id) final override;

private:

	math::float3 color_;

	std::shared_ptr<image::texture::Texture_2D> normal_;

	float sqrt_roughness_;
};

}}}

