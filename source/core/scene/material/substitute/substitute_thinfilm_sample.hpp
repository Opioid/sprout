#pragma once

#include "substitute_base_sample.hpp"

namespace scene { namespace material { namespace substitute {

class Sample_thinfilm : public Sample_base {

public:

	virtual math::float3 evaluate(math::pfloat3 wi, float& pdf) const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler,
								 bxdf::Result& result) const final override;

	void set(math::pfloat3 color, math::pfloat3 radiance,
			 float constant_f0, float ior, float a2, float metallic,
			 float thinfilm_ior, float thinfilm_thickness);

private:

	void diffuse_importance_sample_and_thinfilm(sampler::Sampler& sampler,
												bxdf::Result& result) const;

	void specular_importance_sample_and_thinfilm(sampler::Sampler& sampler,
												 bxdf::Result& result) const;

	void pure_specular_importance_sample_and_thinfilm(sampler::Sampler& sampler,
													  bxdf::Result& result) const;

	float ior_;
	float thinfilm_ior_;
	float thinfilm_thickness_;
};

}}}
