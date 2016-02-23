#pragma once

#include "scene/material/material_sample.hpp"
#include "scene/material/ggx/ggx.hpp"

namespace scene { namespace material { namespace display {

class Sample : public material::Sample {
public:

	virtual math::vec3 evaluate(math::pvec3 wi, float& pdf) const final override;

	virtual math::vec3 emission() const final override;

	virtual math::vec3 attenuation() const final override;

	virtual float ior() const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const final override;

	virtual bool is_pure_emissive() const final override;

	virtual bool is_transmissive() const final override;

	virtual bool is_translucent() const final override;

	void set(const math::vec3& emission, float f0, float roughness);

private:

	math::vec3 emission_;
	math::vec3 f0_;
	float a2_;

	ggx::Schlick_isotropic<Sample> ggx_;
	friend ggx::Schlick_isotropic<Sample>;
};

}}}
