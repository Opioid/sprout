#include "volumetric_sample.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "base/math/vector3.inl"
#include "base/math/sampling/sampling.hpp"

#include <iostream>

namespace scene::material::volumetric {

const material::Sample::Layer& Sample::base_layer() const {
	return layer_;
}

bxdf::Result Sample::evaluate(const float3& wi) const {
	const float phase = layer_.phase(wo_, -wi);

	return { float3(phase), phase };
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Sample& result) const {
/*	const float2 uv = sampler.generate_sample_2D();
	const float3 dir = math::sample_sphere_uniform(uv);

	const float phase = layer_.phase(wo_, -dir);

	result.reflection = float3(phase);
	result.wi = dir;
	result.pdf = 1.f / (4.f * math::Pi);//phase;
	result.type.clear(bxdf::Type::Diffuse_reflection);
	*/

	const float2 r2 = sampler.generate_sample_2D();
	const float3 dir = layer_.sample(wo_, r2);

	const float phase = layer_.phase(wo_, -dir);

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

void Sample::set(const float3& absorption_coefficient,
				 const float3& scattering_coefficient,
				 float anisotropy) {
	layer_.bssrdf.set(absorption_coefficient, scattering_coefficient, anisotropy);
}

float Sample::Layer::phase(const float3& w, const float3& wp) const {
	const float g = bssrdf.anisotropy();
	return phase_hg(w, wp, g);
//	const float k = 1.55f * g - (0.55f * g) * (g * g);
//	return phase_schlick(w, wp, k);
}

float3 Sample::Layer::sample(const float3& w, float2 r2) const {
	const float g = bssrdf.anisotropy();

	float cos_theta;
	if (std::abs(g) < 0.001f) {
		cos_theta = 1.f - 2.f * r2[0];
	} else {
		const float gg = g * g;
		const float sqr = (1.f - gg) / (1.f - g + 2.f * g * r2[0]);
		cos_theta = (1.f + gg - sqr * sqr) / (2.f * g);
	}

	const float sin_theta = std::sqrt(std::max(0.f, 1.f - cos_theta * cos_theta));
	const float phi = r2[1] * (2.f * math::Pi);
	float3 t, b;
	math::orthonormal_basis(w, t, b);
	return math::sphere_direction(sin_theta, cos_theta, phi, t, b, w);
}

float Sample::phase_hg(const float3& w, const float3& wp, float g) {
	const float cos_theta = math::dot(w, wp);
	const float gg = g * g;
	return (1.f / (4.f * math::Pi)) * (1.f - gg) / math::pow1_5(1.f + gg - 2.f * g * cos_theta);
}

float Sample::phase_schlick(const float3& w, const float3& wp, float k) {
	const float d = 1.f - (k * math::dot(w, wp));
	return 1.f / (4.f * math::Pi) * (1.f - k * k) / (d * d);
}

}
