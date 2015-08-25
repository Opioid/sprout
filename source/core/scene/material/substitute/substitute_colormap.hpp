#pragma once

#include "substitute.hpp"
#include "image/texture/texture_2d.hpp"

namespace scene { namespace material { namespace substitute {

template<bool Two_sided, bool Thin>
class Colormap : public Substitute {
public:

	Colormap(Sample_cache& cache,
			 std::shared_ptr<image::texture::Texture_2D> mask,
			 std::shared_ptr<image::texture::Texture_2D> color,
			 float roughness,
			 float metallic);

	virtual const material::Sample& sample(const shape::Differential& dg, const math::float3& wo,
										   const image::texture::sampler::Sampler_2D& sampler,
										   uint32_t worker_id) final override;

private:

	std::shared_ptr<image::texture::Texture_2D> color_;
	float roughness_;
	float metallic_;
};

}}}
