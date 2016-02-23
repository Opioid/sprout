#pragma once

#include "scene/material/material_sample.hpp"
#include "scene/material/ggx/ggx.hpp"

namespace scene { namespace material { namespace metal {

class Sample_isotropic : public material::Sample {
public:

	virtual math::vec3 evaluate(math::pvec3 wi, float& pdf) const final override;

	virtual math::vec3 emission() const final override;

	virtual math::vec3 attenuation() const final override;

	virtual float ior() const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const final override;

	virtual bool is_pure_emissive() const final override;

	virtual bool is_transmissive() const final override;

	virtual bool is_translucent() const final override;

	void set(const math::vec3& ior, const math::vec3& absorption, float roughness);

private:

	math::vec3 ior_;
	math::vec3 absorption_;

	float a2_;

	ggx::Conductor_isotropic<Sample_isotropic> ggx_;

	friend ggx::Conductor_isotropic<Sample_isotropic>;
};

class Sample_anisotropic : public material::Sample {
public:

	virtual math::vec3 evaluate(math::pvec3 wi, float& pdf) const final override;

	virtual math::vec3 emission() const final override;

	virtual math::vec3 attenuation() const final override;

	virtual float ior() const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const final override;

	virtual bool is_pure_emissive() const final override;

	virtual bool is_transmissive() const final override;

	virtual bool is_translucent() const final override;

	void set(const math::vec3& ior, const math::vec3& absorption, math::float2 roughness);

private:

	math::vec3 ior_;
	math::vec3 absorption_;
	math::float2 a_;
	math::float2 a2_;
	float axy_;

	ggx::Conductor_anisotropic<Sample_anisotropic> ggx_;

	friend ggx::Conductor_anisotropic<Sample_anisotropic>;
};

}}}

