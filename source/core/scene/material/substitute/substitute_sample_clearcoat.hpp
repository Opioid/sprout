#pragma once

#include "substitute_sample_base.hpp"

namespace scene { namespace material { namespace substitute {

class Sample_clearcoat : public Sample_base {
public:

	virtual math::float3 evaluate(math::pfloat3 wi, float& pdf) const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const final override;

	void set(const math::float3& color, const math::float3& emission,
			 float constant_f0, float a2, float metallic,
			 float clearcoat_ior, float clearcoat_a2);

private:

	void diffuse_importance_sample_and_clearcoat(sampler::Sampler& sampler, bxdf::Result& result) const;
	void specular_importance_sample_and_clearcoat(sampler::Sampler& sampler, bxdf::Result& result) const;
	void pure_specular_importance_sample_and_clearcoat(sampler::Sampler& sampler, bxdf::Result& result) const;

	float clearcoat_ior_;
	float clearcoat_f0_;
	float clearcoat_a2_;
};

}}}
