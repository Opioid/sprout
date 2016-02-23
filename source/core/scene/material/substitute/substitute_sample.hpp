#pragma once

#include "scene/material/material_sample.hpp"
#include "scene/material/ggx/ggx.hpp"
#include "scene/material/lambert/lambert.hpp"
#include "scene/material/oren_nayar/oren_nayar.hpp"

namespace scene { namespace material { namespace substitute {

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

	void set(const math::vec3& color, const math::vec3& emission,
			 float constant_f0, float roughness, float metallic,
			 float thickness, float attenuation_distance);

private:

	math::vec3 diffuse_color_;
	math::vec3 attenuation_;
	math::vec3 f0_;
	math::vec3 emission_;

	float a2_;
	float metallic_;
	float thickness_;

	lambert::Lambert<Sample> lambert_;
	oren_nayar::Oren_nayar<Sample> oren_nayar_;
	ggx::Schlick_isotropic<Sample> ggx_;

	friend lambert::Lambert<Sample>;
	friend oren_nayar::Oren_nayar<Sample>;
	friend ggx::Schlick_isotropic<Sample>;
};

}}}
