#pragma once

#include "light_material.hpp"
#include "image/texture/texture_2d.hpp"
#include "base/math/distribution_2d.hpp"

namespace scene { namespace material { namespace light {

class Emissionmap : public Light {
public:

	Emissionmap(Sample_cache<Sample>& cache, std::shared_ptr<image::Image> mask, std::shared_ptr<image::Image> emission);

	virtual const Sample& sample(const shape::Differential& dg, const math::float3& wo,
								 const image::sampler::Sampler_2D& sampler, uint32_t worker_id) final override;

	virtual math::float3 sample_emission(math::float2 uv, const image::sampler::Sampler_2D& sampler) const final override;

	virtual math::float3 average_emission() const final override;

	virtual const image::Texture_2D* emission_map() const final override;

	virtual math::float2 emission_importance_sample(math::float2 r2, float& pdf) const final override;

	virtual void prepare_sampling() final override;

private:

	image::Texture_2D emission_;

	math::Distribution_2D distribution_;

	float num_pixels_;
};

}}}
