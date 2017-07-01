#pragma once

#include "scene/material/material_sample.hpp"

namespace scene { namespace material { namespace metal {

class Sample_isotropic : public material::Sample {

public:

	virtual const Layer& base_layer() const final override;

	virtual float3 evaluate(const float3& wi, float& pdf) const final override;

	virtual float ior() const final override;

	virtual void sample(sampler::Sampler& sampler, bxdf::Result& result) const final override;

	struct Layer : material::Sample::Layer {
		void set(const float3& ior, const float3& absorption, float roughness);
		float3 ior_;
		float3 absorption_;
		float a2_;
	};

	Layer layer_;
};

class Sample_anisotropic : public material::Sample {

public:

	virtual const Layer& base_layer() const final override;

	virtual float3 evaluate(const float3& wi, float& pdf) const final override;

	virtual float ior() const final override;

	virtual void sample(sampler::Sampler& sampler, bxdf::Result& result) const final override;

	struct Layer : material::Sample::Layer {
		void set(const float3& ior, const float3& absorption, float2 roughness);

		float3 ior_;
		float3 absorption_;
		float2 a_;
		float2 a2_;
		float axy_;
	};

	Layer layer_;
};

}}}

