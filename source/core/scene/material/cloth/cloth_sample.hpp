#pragma once

#include "scene/material/material_sample.hpp"
#include "scene/material/lambert/lambert.hpp"

namespace scene { namespace material { namespace cloth {

class Sample : public material::Sample {

public:

	virtual math::float3 evaluate(math::pfloat3 wi, float& pdf) const final override;

	virtual math::float3 radiance() const final override;

	virtual math::float3 attenuation() const final override;

	virtual float ior() const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const final override;

	virtual bool is_pure_emissive() const final override;

	virtual bool is_transmissive() const final override;

	virtual bool is_translucent() const final override;

	void set(math::pfloat3 color);

private:

	math::float3 diffuse_color_;

	friend lambert::Isotropic;
};

}}}
