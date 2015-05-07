#pragma once

#include "substitute.hpp"
#include "image/texture/texture_2d.hpp"

namespace scene { namespace material { namespace substitute {

class Colormap : public Substitute {
public:

	Colormap(Sample_cache<Sample>& cache, std::shared_ptr<image::Image> mask,
			 std::shared_ptr<image::Image> color, float roughness, float metallic);

	virtual const Sample& sample(const shape::Differential& dg, const math::float3& wo,
								 const image::sampler::Sampler_2D& sampler, uint32_t worker_id);

private:

	image::Texture_2D color_;
	float roughness_;
	float metallic_;
};

}}}
