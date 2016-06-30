#pragma once

#include "substitute_base_material.hpp"

namespace scene { namespace material { namespace substitute {

class Sample_translucent;

class Material_translucent : public Material_base<Sample_translucent> {

public:

	Material_translucent(Generic_sample_cache<Sample_translucent>& cache,
						 const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(float3_p wo, const Renderstate& rs,
										   const Worker& worker,
										   Sampler_filter filter) final override;

	void set_thickness(float thickness);
	void set_attenuation_distance(float attenuation_distance);

private:

	float thickness_;
	float attenuation_distance_;
};

}}}
