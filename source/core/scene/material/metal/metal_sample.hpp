#pragma once

#include "scene/material/material_sample.hpp"
#include "scene/material/ggx/ggx.hpp"

namespace scene { namespace material { namespace metal {

class Sample;


class Sample : public material::Sample {
public:

	Sample();

	virtual math::float3 evaluate(const math::float3& wi, float& pdf) const final override;

	virtual math::float3 emission() const final override;

	virtual math::float3 attenuation() const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const final override;

	virtual bool is_pure_emissive() const final override;

	virtual bool is_translucent() const final override;

	void set(const math::float3& ior, const math::float3& absorption, float sqrt_roughness);

private:

	math::float3 ior_;
	math::float3 absorption_;

	float a2_;

	ggx::GGX_Conductor<Sample> ggx_;

	friend ggx::GGX_Conductor<Sample>;
};

}}}

