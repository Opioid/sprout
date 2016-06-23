#pragma once

#include "substitute_base_material.hpp"

namespace scene { namespace material { namespace substitute {

class Sample;

class Material : public Material_base<Sample> {

public:

	Material(Generic_sample_cache<Sample>& cache,
			 Texture_2D_ptr mask,
			 const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(const shape::Hitpoint& hp, float3_p wo,
										   float area, float time, float ior_i,
										   const Worker& worker,
										   Sampler_filter filter) final override;
};

}}}
