#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"
#include "scene/material/light/light_material_sample.hpp"

namespace scene { namespace material { namespace sky {

class Material_overcast : public Material {

public:

	Material_overcast(const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(const float3& wo, const Renderstate& rs,
										   Worker& worker, Sampler_filter filter) override final;

	virtual float3 sample_radiance(const float3& wi, float2 uv, float area, float time,
								   Worker& worker, Sampler_filter filter) const override final;

	virtual float3 average_radiance(float area) const override final;

	virtual size_t num_bytes() const override final;

	void set_emission(const float3& radiance);

private:

	float3 overcast(const float3& wi) const;

	float3 color_;
};

}}}

