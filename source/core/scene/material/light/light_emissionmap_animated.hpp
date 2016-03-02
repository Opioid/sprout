#pragma once

#include "light_material.hpp"
#include "image/texture/texture_2d.hpp"
#include "base/math/distribution/distribution_2d.hpp"

namespace scene { namespace material { namespace light {

class Sample;

class Emissionmap_animated : public Material {
public:

	Emissionmap_animated(Generic_sample_cache<Sample>& cache,
						 std::shared_ptr<image::texture::Texture_2D> mask, bool two_sided,
						 std::shared_ptr<image::texture::Texture_2D> emission_map,
						 float emission_factor, float animation_duration);

	virtual void tick(float absolute_time, float time_slice) final override;

	virtual const material::Sample& sample(const shape::Differential& dg, const math::float3& wo,
										   float time, float ior_i,
										   const image::texture::sampler::Sampler_2D& sampler,
										   uint32_t worker_id) final override;

	virtual math::float3 sample_emission(math::float2 uv, float time,
										 const image::texture::sampler::Sampler_2D& sampler) const final override;

	virtual math::float3 average_emission() const final override;

	virtual bool has_emission_map() const final override;

	virtual math::float2 emission_importance_sample(math::float2 r2, float& pdf) const final override;

	virtual float emission_pdf(math::float2 uv,
							   const image::texture::sampler::Sampler_2D& sampler) const final override;

	virtual float opacity(math::float2 uv, float time,
						  const image::texture::sampler::Sampler_2D& sampler) const final override;

	virtual void prepare_sampling(bool spherical) final override;

	virtual bool is_animated() const final override;

private:

	std::shared_ptr<image::texture::Texture_2D> emission_map_;

	float emission_factor_;

	std::vector<math::float3> average_emissions_;

	float total_weight_;

	math::Distribution_2D distribution_;

	const float frame_length_;

	int32_t element_;
};

}}}
