#pragma once

#include "light_material.hpp"
#include "base/math/distribution/distribution_2d.hpp"

namespace scene { namespace material { namespace light {

class Sample;

class Emissionmap : public Material {

public:

	Emissionmap(Generic_sample_cache<Sample>& cache,
				const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(float3_p wo, const Renderstate& rs,
										   const Worker& worker,
										   Sampler_filter filter) final override;

	virtual float3 sample_radiance(float3_p wi, float2 uv, float area,
								   float time, const Worker& worker,
								   Sampler_filter filter) const final override;

	virtual float3 average_radiance(float area) const final override;

	virtual bool has_emission_map() const final override;

	virtual float2 radiance_sample(float2 r2, float& pdf) const final override;

	virtual float emission_pdf(float2 uv, const Worker& worker,
							   Sampler_filter filter) const final override;

	virtual void prepare_sampling(const shape::Shape& shape, uint32_t part,
								  const Transformation& transformation,
								  float area, bool importance_sampling,
								  thread::Pool& pool) final override;

	virtual size_t num_bytes() const final override;

	void set_emission_map(const Adapter_2D& emission_map);
	void set_emission_factor(float emission_factor);

private:

	Adapter_2D emission_map_;

	float emission_factor_;

	float3 average_emission_;

	float total_weight_;

	math::Distribution_2D distribution_;
};

}}}
