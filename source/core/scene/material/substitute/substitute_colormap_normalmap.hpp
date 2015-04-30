#pragma once

#include "substitute.hpp"
#include "image/texture/texture_2d.hpp"

namespace scene { namespace material { namespace substitute {

class Colormap_normalmap : public Substitute {
public:

	Colormap_normalmap(Sample_cache<Sample>& cache, std::shared_ptr<image::Image> color, std::shared_ptr<image::Image> normal,
					   float roughness, float metallic);

	virtual const Sample& sample(const shape::Differential& dg, const math::float3& wo,
								 const image::sampler::Sampler_2D& sampler, uint32_t worker_id) final override;

private:

	image::Texture_2D color_;
	image::Texture_2D normal_;
	float roughness_;
	float metallic_;
};

}}}
