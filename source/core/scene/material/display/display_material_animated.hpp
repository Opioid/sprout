#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"
#include "image/texture/texture_2d.hpp"
#include "base/math/distribution/distribution_2d.hpp"

namespace scene { namespace material { namespace display {

class Sample;

class Material_animated : public material::Typed_material<Generic_sample_cache<Sample>> {
public:

	Material_animated(Generic_sample_cache<Sample>& cache,
					  std::shared_ptr<image::texture::Texture_2D> mask,
					  const Sampler_settings& sampler_settings,
					  bool two_sided,
					  std::shared_ptr<image::texture::Texture_2D> emission_map,
					  float animation_duration);

	virtual void tick(float absolute_time, float time_slice) final override;

	virtual const material::Sample& sample(const shape::Hitpoint& hp, math::pfloat3 wo,
										   float time, float ior_i,
										   const Worker& worker, Sampler_settings::Filter filter) final override;

	virtual math::float3 sample_emission(math::pfloat3 wi, math::float2 uv, float time,
										 const Worker& worker, Sampler_settings::Filter filter) const final override;

	virtual math::float3 average_emission() const final override;

	virtual bool has_emission_map() const final override;

	virtual math::float2 emission_importance_sample(math::float2 r2, float& pdf) const final override;

	virtual float emission_pdf(math::float2 uv,
							   const Worker& worker, Sampler_settings::Filter filter) const final override;

	virtual float opacity(math::float2 uv, float time,
						  const Worker& worker, Sampler_settings::Filter filter) const final override;

	virtual void prepare_sampling(bool spherical) final override;

	virtual bool is_animated() const final override;

	void set_emission_factor(float emission_factor);
	void set_roughness(float roughness);
	void set_ior(float ior);

private:

	std::shared_ptr<image::texture::Texture_2D> emission_map_;

	math::float3 emission_;

	float emission_factor_;

	float roughness_;

	float f0_;

	std::vector<math::float3> average_emissions_;

	float total_weight_;

	math::Distribution_2D distribution_;

	const float frame_length_;

	int32_t element_;
};

}}}
