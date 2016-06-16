#pragma once

#include "substitute_base_sample.hpp"
#include "scene/material/coating/coating.hpp"

namespace scene { namespace material { namespace substitute {

class Sample_clearcoat : public Sample_base {

public:

	virtual math::float3 evaluate(math::pfloat3 wi, float& pdf) const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler,
								 bxdf::Result& result) const final override;

	void set_clearcoat(const coating::Clearcoat& clearcoat);

private:

	void diffuse_importance_sample_and_clearcoat(sampler::Sampler& sampler,
												 bxdf::Result& result) const;

	void specular_importance_sample_and_clearcoat(sampler::Sampler& sampler,
												  bxdf::Result& result) const;

	void pure_specular_importance_sample_and_clearcoat(sampler::Sampler& sampler,
													   bxdf::Result& result) const;

	coating::Clearcoat clearcoat_;
};

}}}
