#pragma once

#include "scene/material/material_sample.hpp"
#include "scene/material/bxdf.hpp"

namespace scene { namespace material { namespace sky {

class Model;

class Sample_clear : public material::Sample {
public:

	virtual math::float3 evaluate(math::pfloat3 wi, float& pdf) const final override;

	virtual math::float3 emission() const final override;

	virtual math::float3 attenuation() const final override;

	virtual float ior() const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler,
								 bxdf::Result& result) const final override;

	virtual bool is_pure_emissive() const final override;

	virtual bool is_transmissive() const final override;

	virtual bool is_translucent() const final override;

	void set(const Model* model, math::pfloat3 zenith);

private:

	const Model* model_;

	math::float3 zenith_;

	math::float3 emission_;
};

}}}
