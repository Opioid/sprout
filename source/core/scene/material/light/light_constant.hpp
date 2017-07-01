#pragma once

#include "scene/material/material.hpp"
#include "scene/light/emittance.hpp"

namespace scene { namespace material { namespace light {

class Constant : public Material {

public:

	Constant(const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(const float3& wo, const Renderstate& rs,
										   Worker& worker, Sampler_filter filter) final override;

	virtual float3 sample_radiance(const float3& wi, float2 uv, float area, float time,
								   Worker& worker, Sampler_filter filter) const final override;

	virtual float3 average_radiance(float area) const final override;

	virtual bool has_emission_map() const final override;

	virtual size_t num_bytes() const final override;

	::light::Emittance& emittance();

private:

	::light::Emittance emittance_;
};

}}}
