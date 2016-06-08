#pragma once

#include "scene/material/material_sample.hpp"

namespace scene { namespace material {

namespace bxdf { struct Result; }

namespace glass {

class Sample;

class BRDF {

public:

	static float importance_sample(const Sample& sample, sampler::Sampler& sampler,
								   bxdf::Result& result);
};

class BTDF {

public:

	static float importance_sample(const Sample& sample, sampler::Sampler& sampler,
								   bxdf::Result& result);
};

class Sample : public material::Sample {

public:

	virtual math::float3 evaluate(math::pfloat3 wi, float& pdf) const final override;

	virtual math::float3 radiance() const final override;

	virtual math::float3 attenuation() const final override;

	virtual float ior() const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler,
								 bxdf::Result& result) const final override;

	virtual bool is_pure_emissive() const final override;

	virtual bool is_transmissive() const final override;

	virtual bool is_translucent() const final override;

	void set(math::pfloat3 color, float attenuation_distance, float ior, float ior_outside);

private:

	math::float3 color_;
	math::float3 attenuation_;
	float ior_;
	float ior_outside_;

	friend BRDF;
	friend BTDF;
};

}}}
