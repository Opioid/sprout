#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"

namespace scene { namespace material { namespace sky {

class Sample_overcast;

class Material_overcast : public material::Typed_material<Generic_sample_cache<Sample_overcast>> {
public:

	Material_overcast(Generic_sample_cache<Sample_overcast>& cache,
					  std::shared_ptr<image::texture::Texture_2D> mask,
					  const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(const shape::Hitpoint& hp, math::pfloat3 wo,
										   float time, float ior_i,
										   const Worker& worker, Sampler_settings::Filter filter) final override;

	virtual math::float3 sample_emission(math::float2 uv, float time,
										 const Worker& worker, Sampler_settings::Filter filter) const final override;

	virtual math::float3 average_emission() const final override;

	virtual bool has_emission_map() const final override;

	void set_emission(math::pfloat3 emission);

protected:

	math::float3 emission_;
};

}}}

