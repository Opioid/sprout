#pragma once

#include "substitute.hpp"
#include "image/texture/texture_2d.hpp"

namespace scene { namespace material { namespace substitute {

class Colormap_surfacemap : public Substitute {
public:

	Colormap_surfacemap(Sample_cache<Sample>& cache,
						std::shared_ptr<image::texture::Texture_2D> mask,
						bool two_sided,
						std::shared_ptr<image::texture::Texture_2D> color,
						std::shared_ptr<image::texture::Texture_2D>surface);

	virtual const Sample& sample(const shape::Differential& dg, const math::float3& wo,
								 const image::texture::sampler::Sampler_2D& sampler,
								 uint32_t worker_id) final override;

private:

	std::shared_ptr<image::texture::Texture_2D> color_;
	std::shared_ptr<image::texture::Texture_2D> surface_;
};

}}}
