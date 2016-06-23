#pragma once

#include "light_material.hpp"
#include "image/texture/texture_2d.hpp"
#include "base/math/distribution/distribution_2d.hpp"

namespace scene { namespace material { namespace light {

class Sample;

class Emissionmap : public Material {
public:

	Emissionmap(Generic_sample_cache<Sample>& cache,
				std::shared_ptr<image::texture::Texture_2D> mask,
				const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(const shape::Hitpoint& hp, float3_p wo,
										   float area, float time, float ior_i,
										   const Worker& worker,
										   Sampler_filter filter) final override;

	virtual float3 sample_radiance(float3_p wi, float2 uv,
										 float area, float time, const Worker& worker,
										 Sampler_filter filter) const final override;

	virtual float3 average_radiance(float area) const final override;

	virtual bool has_emission_map() const final override;

	virtual float2 radiance_importance_sample(float2 r2,
													float& pdf) const final override;

	virtual float emission_pdf(float2 uv, const Worker& worker,
							   Sampler_filter filter) const final override;

	virtual void prepare_sampling(bool spherical) final override;

	void set_emission_map(std::shared_ptr<image::texture::Texture_2D> emission_map);
	void set_emission_factor(float emission_factor);

private:

	std::shared_ptr<image::texture::Texture_2D> emission_map_;

	float emission_factor_;

	float3 average_emission_;

	float total_weight_;

	math::Distribution_2D distribution_;
};

}}}
