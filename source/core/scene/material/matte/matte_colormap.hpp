#pragma once

#include "matte.hpp"
#include "image/texture/texture_2d.hpp"

namespace scene { namespace material { namespace matte {

template<bool Two_sided>
class Colormap : public Matte {
public:

	Colormap(Sample_cache<Sample>& cache,
			 std::shared_ptr<image::texture::Texture_2D> mask,
			 std::shared_ptr<image::texture::Texture_2D> color,
			 float sqrt_roughness);

	virtual const Sample& sample(const shape::Differential& dg, const math::float3& wo,
								 const image::texture::sampler::Sampler_2D& sampler,
								 uint32_t worker_id) final override;

private:

	std::shared_ptr<image::texture::Texture_2D> color_;

	float sqrt_roughness_;
};

}}}

