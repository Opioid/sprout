#include "volumetric_sample.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "base/math/vector3.inl"
#include "base/math/sampling/sampling.hpp"

namespace scene::material::volumetric {

const material::Sample::Layer& Sample::base_layer() const {
	return layer_;
}

bxdf::Result Sample::evaluate(const float3& wi) const {
	const float phase = layer_.phase(wo_, wi);

	return { float3(phase), phase };
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Sample& result) const {
	const float2 uv = sampler.generate_sample_2D();
	const float3 dir = math::sample_sphere_uniform(uv);

	const float phase = layer_.phase(wo_, dir);

	result.reflection = float3(phase);
	result.wi = dir;
	result.pdf = phase;
	result.type.clear(bxdf::Type::Diffuse_reflection);
}

BSSRDF Sample::bssrdf() const {
	return layer_.bssrdf;
}

float Sample::ior() const {
	return 1.f;
}

bool Sample::is_translucent() const {
	return true;
}

float Sample::Layer::phase(const float3& w, const float3& wp) const {
	const float g = bssrdf.anisotropy();
	const float k = 1.55f * g - (0.55f * g) * (g * g);
	return phase_schlick(w, wp, k);
}

float Sample::phase_schlick(const float3& w, const float3& wp, float k) {
	const float d = 1.f - (k * math::dot(w, wp));
	return 1.f / (4.f * math::Pi) * (1.f - k * k) / (d * d);
}


}
