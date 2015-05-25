#pragma once

#include "matte.hpp"
#include "image/texture/texture_2d.hpp"

namespace scene { namespace material { namespace matte {

class Colormap_normalmap : public Matte {
public:

	Colormap_normalmap(Sample_cache<Sample>& cache, std::shared_ptr<image::Image> mask,
					   std::shared_ptr<image::Image> color, std::shared_ptr<image::Image> normal);

	virtual const Sample& sample(const shape::Differential& dg, const math::float3& wo,
								 const image::sampler::Sampler_2D& sampler, uint32_t worker_id) final override;

private:

	image::Texture_2D color_;
	image::Texture_2D normal_;
};

}}}

