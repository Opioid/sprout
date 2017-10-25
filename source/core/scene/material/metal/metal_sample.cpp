#include "metal_sample.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/ggx/ggx.inl"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"

namespace scene::material::metal {

const material::Sample::Layer& Sample_isotropic::base_layer() const {
	return layer_;
}

bxdf::Result Sample_isotropic::evaluate(const float3& wi) const {
	if (!same_hemisphere(wo_)) {
		return { float3::identity(), 0.f };
	}

	const float n_dot_wi = layer_.clamp_n_dot(wi);
	const float n_dot_wo = layer_.clamp_abs_n_dot(wo_); //layer_.clamp_n_dot(wo_);

	const float3 h = math::normalize(wo_ + wi);
	const float wo_dot_h = clamp_dot(wo_, h);

	const float n_dot_h = math::saturate(math::dot(layer_.n_, h));

	const fresnel::Conductor conductor(layer_.ior_, layer_.absorption_);
	float pdf;
	const float3 reflection = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h,
														 layer_, conductor, pdf);

	return { n_dot_wi * reflection, pdf };
}

float Sample_isotropic::ior() const {
	return 1.5f;
}

void Sample_isotropic::sample(sampler::Sampler& sampler, bxdf::Sample& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	const float n_dot_wo = layer_.clamp_abs_n_dot(wo_); //layer_.clamp_n_dot(wo_);

	const fresnel::Conductor conductor(layer_.ior_, layer_.absorption_);
	const float n_dot_wi = ggx::Isotropic::reflect(wo_, n_dot_wo, layer_, conductor,
												   sampler, result);
	result.reflection *= n_dot_wi;
}

void Sample_isotropic::Layer::set(const float3& ior, const float3& absorption, float roughness) {
	ior_ = ior;
	absorption_ = absorption;
	const float alpha = roughness * roughness;
	alpha_ = alpha;
	alpha2_ = alpha * alpha;
}

const material::Sample::Layer& Sample_anisotropic::base_layer() const {
	return layer_;
}

bxdf::Result Sample_anisotropic::evaluate(const float3& wi) const {
	if (!same_hemisphere(wo_)) {
		return { float3::identity(), 0.f };
	}

	const float n_dot_wi = layer_.clamp_n_dot(wi);
	const float n_dot_wo = layer_.clamp_abs_n_dot(wo_); //layer_.clamp_n_dot(wo_);

	const float3 h = math::normalize(wo_ + wi);
	const float wo_dot_h = clamp_dot(wo_, h);

	const fresnel::Conductor conductor(layer_.ior_, layer_.absorption_);
	float pdf;
	const float3 reflection = ggx::Anisotropic::reflection(h, n_dot_wi, n_dot_wo, wo_dot_h,
														   layer_, conductor, pdf);

	return { n_dot_wi * reflection, pdf };
}

float Sample_anisotropic::ior() const {
	return 1.5f;
}

void Sample_anisotropic::sample(sampler::Sampler& sampler, bxdf::Sample& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	const float n_dot_wo = layer_.clamp_abs_n_dot(wo_); //layer_.clamp_n_dot(wo_);

	const fresnel::Conductor conductor(layer_.ior_, layer_.absorption_);
	const float n_dot_wi = ggx::Anisotropic::reflect(wo_, n_dot_wo, layer_, conductor,
													 sampler, result);
	result.reflection *= n_dot_wi;
}

void Sample_anisotropic::Layer::set(const float3& ior, const float3& absorption, float2 roughness) {
	ior_ = ior;
	absorption_ = absorption;

	const float2 a = roughness * roughness;
	a_   = a;
	alpha2_  = a * a;
	axy_ = a[0] * a[1];
}

}
