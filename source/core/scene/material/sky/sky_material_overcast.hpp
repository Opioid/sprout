#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"
#include "scene/material/light/light_material_sample.hpp"

namespace scene { namespace material { namespace sky {

class Material_overcast : public material::Typed_material<Generic_sample_cache<light::Sample>> {
public:

	Material_overcast(Generic_sample_cache<light::Sample>& cache,
					  Texture_2D_ptr mask,
					  const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(const shape::Hitpoint& hp, float3_p wo,
										   float area, float time, float ior_i,
										   const Worker& worker,
										   Sampler_filter filter) final override;

	virtual float3 sample_radiance(float3_p wi, float2 uv,
										 float area, float time, const Worker& worker,
										 Sampler_filter filter) const final override;

	virtual float3 average_radiance(float area) const final override;

	void set_emission(float3_p radiance);

private:

	float3 overcast(float3_p wi) const;

	float3 color_;
};

}}}

