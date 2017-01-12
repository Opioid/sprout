#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"
#include "scene/light/emittance.hpp"

namespace scene { namespace material { namespace light {

class Sample;

class Constant : public Typed_material<Sample_cache<Sample>> {

public:

	Constant(BSSRDF_cache& bssrdf_cache, const Sampler_settings& sampler_settings,
			 bool two_sided, Sample_cache<Sample>& cache);

	virtual const material::Sample& sample(float3_p wo, const Renderstate& rs,
										   const Worker& worker,
										   Sampler_filter filter) final override;

	virtual float3 sample_radiance(float3_p wi, float2 uv, float area, float time,
								   const Worker& worker,
								   Sampler_filter filter) const final override;

	virtual float3 average_radiance(float area) const final override;

	virtual bool has_emission_map() const final override;

	virtual size_t num_bytes() const final override;

	::light::Emittance& emittance();

private:

	::light::Emittance emittance_;
};

}}}
