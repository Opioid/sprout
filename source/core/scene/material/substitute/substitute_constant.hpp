#pragma once

#include "substitute.hpp"

namespace scene { namespace material { namespace substitute {

template<bool Two_sided>
class Constant : public Substitute {
public:

	Constant(Sample_cache<Sample>& cache,
			 std::shared_ptr<image::texture::Texture_2D> mask,
			 const math::float3& color,
			 float roughness,
			 float metallic);

	virtual const Sample& sample(const shape::Differential& dg, const math::float3& wo,
								 const image::texture::sampler::Sampler_2D& sampler,
								 uint32_t worker_id) final override;

private:

	math::float3 color_;
	float roughness_;
	float metallic_;
};

}}}
