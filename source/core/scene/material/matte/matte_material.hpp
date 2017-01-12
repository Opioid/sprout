#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"

namespace scene { namespace material { namespace matte {

class Sample;

class Material : public Typed_material<Sample_cache<Sample>> {

public:

	Material(Sample_cache2& sample_cache, const Sampler_settings& sampler_settings,
			 bool two_sided, Sample_cache<Sample>& cache);

	virtual const material::Sample& sample(float3_p wo, const Renderstate& rs,
										   const Worker& worker,
										   Sampler_filter filter) final override;

	virtual size_t num_bytes() const final override;

	void set_color(float3_p color);

private:

	float3 color_;
};

}}}
