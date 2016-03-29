#pragma once

#include "substitute_sample_base.hpp"

namespace scene { namespace material { namespace substitute {

class Sample : public Sample_base {
public:

	virtual math::float3 evaluate(math::pfloat3 wi, float& pdf) const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const final override;

	void set(const math::float3& color, const math::float3& emission,
			 float constant_f0, float roughness, float metallic);

};

}}}
