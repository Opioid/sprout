#pragma once

#include "scene/material/material_sample.hpp"

namespace scene { namespace material {

namespace bxdf { struct Result; }

namespace glass {

class Sample;

class BRDF {
public:

	math::vec3 evaluate(const Sample& sample, const math::vec3& wi, float n_dot_wi) const;

	float pdf(const Sample& sample, const math::vec3& wi, float n_dot_wi) const;

	float importance_sample(const Sample& sample, sampler::Sampler& sampler, bxdf::Result& result) const;
};

class BTDF {
public:

	math::vec3 evaluate(const Sample& sample, const math::vec3& wi, float n_dot_wi) const;

	float pdf(const Sample& sample, const math::vec3& wi, float n_dot_wi) const;

	float importance_sample(const Sample& sample, sampler::Sampler& sampler, bxdf::Result& result) const;
};

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

	void set(const math::vec3& color, float attenuation_distance, float ior, float ior_outside);

private:

	math::vec3 color_;
	math::vec3 attenuation_;
	float ior_;
	float ior_outside_;

	BRDF brdf_;
	BTDF btdf_;

	friend BRDF;
	friend BTDF;
};

}}}
