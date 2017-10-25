#pragma once

#include "scene/material/material.hpp"
#include "scene/light/emittance.hpp"

namespace scene::material::light {

class Constant : public Material {

public:

	Constant(const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(const float3& wo, const Renderstate& rs,
										   Sampler_filter filter,
										   const Worker& worker) override final;

	virtual float3 sample_radiance(const float3& wi, float2 uv, float area, float time,
								   Sampler_filter filter,
								   const Worker& worker) const override final;

	virtual float3 average_radiance(float area) const override final;

	virtual bool has_emission_map() const override final;

	virtual size_t num_bytes() const override final;

	::light::Emittance& emittance();

private:

	::light::Emittance emittance_;
};

}
