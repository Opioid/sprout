#pragma once

#include "substitute_sample_base.hpp"

namespace scene { namespace material { namespace substitute {

class Sample_clearcoat;

class Clearcoat {
public:

	static math::float3 evaluate(const Sample_clearcoat& sample, const math::float3& wi, float n_dot_wi);

	static float pdf(const Sample_clearcoat& sample, const math::float3& wi, float n_dot_wi);

	static float importance_sample(const Sample_clearcoat& sample, sampler::Sampler& sampler, bxdf::Result& result);

	static float fresnel(const Sample_clearcoat& sample);
};

class Sample_clearcoat : public Sample_base {
public:

	virtual math::float3 evaluate(math::pfloat3 wi, float& pdf) const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const final override;

	void set(const math::float3& color, const math::float3& emission,
			 float constant_f0, float roughness, float metallic);

private:

	float clearcoat_ior_;

	friend Clearcoat;
};

}}}
