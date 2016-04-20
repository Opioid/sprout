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

	virtual const material::Sample& sample(const shape::Hitpoint& hp, math::pfloat3 wo,
										   float time, float ior_i, const Worker& worker,
										   Sampler_settings::Filter filter) final override;

	virtual math::float3 sample_emission(math::pfloat3 wi, math::float2 uv,
										 float time, const Worker& worker,
										 Sampler_settings::Filter filter) const final override;

	virtual math::float3 average_emission() const final override;

	virtual bool has_emission_map() const final override;

	virtual math::float2 emission_importance_sample(math::float2 r2,
													float& pdf) const final override;

	virtual float emission_pdf(math::float2 uv, const Worker& worker,
							   Sampler_settings::Filter filter) const final override;

	virtual void prepare_sampling(bool spherical) final override;

	void set_emission_map(std::shared_ptr<image::texture::Texture_2D> emission_map);
	void set_emission_factor(float emission_factor);

private:

	std::shared_ptr<image::texture::Texture_2D> emission_map_;

	float emission_factor_;

	math::float3 average_emission_;

	float total_weight_;

	math::Distribution_2D distribution_;
};

}}}
