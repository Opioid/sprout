#pragma once

#include "substitute_base_material.hpp"

namespace scene { namespace material { namespace substitute {

class Sample;

class Material : public Material_base {

public:

	Material(Sample_cache& sample_cache, const Sampler_settings& sampler_settings,
			 bool two_sided);

	virtual const material::Sample& sample(float3_p wo, const Renderstate& rs,
										   const Worker& worker,
										   Sampler_filter filter) final override;

	virtual size_t num_bytes() const final override;
};

}}}
