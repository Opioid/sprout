#pragma once

#include "scene/material/material_sample.hpp"

namespace scene { namespace material { namespace metal {

class Sample_isotropic : public material::Sample {

public:

	virtual const Layer& base_layer() const override final;

	virtual float3 evaluate(const float3& wi, float& pdf) const override final;

	virtual float ior() const override final;

	virtual void sample(sampler::Sampler& sampler, bxdf::Result& result) const override final;

	struct Layer : material::Sample::Layer {
		void set(const float3& ior, const float3& absorption, float roughness);
		float3 ior_;
		float3 absorption_;
		float alpha_;
		float alpha2_;
	};

	Layer layer_;
};

class Sample_anisotropic : public material::Sample {

public:

	virtual const Layer& base_layer() const override final;

	virtual float3 evaluate(const float3& wi, float& pdf) const override final;

	virtual float ior() const override final;

	virtual void sample(sampler::Sampler& sampler, bxdf::Result& result) const override final;

	struct Layer : material::Sample::Layer {
		void set(const float3& ior, const float3& absorption, float2 roughness);

		float3 ior_;
		float3 absorption_;
		float2 a_;
		float2 alpha2_;
		float axy_;
	};

	Layer layer_;
};

}}}

