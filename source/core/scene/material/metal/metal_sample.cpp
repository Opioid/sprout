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

float3 Sample_isotropic::evaluate(const float3& wi, float& pdf) const {
	if (!same_hemisphere(wo_)) {
		pdf = 0.f;
		return float3::identity();
	}

	const float n_dot_wi = layer_.clamp_n_dot(wi);
	const float n_dot_wo = layer_.clamp_abs_n_dot(wo_); //layer_.clamp_n_dot(wo_);

	const float3 h = math::normalize(wo_ + wi);
	const float wo_dot_h = clamp_dot(wo_, h);

	const float n_dot_h = math::saturate(math::dot(layer_.n_, h));

	const fresnel::Conductor conductor(layer_.ior_, layer_.absorption_);
	return n_dot_wi * ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h,
												 layer_, conductor, pdf);
}

float Sample_isotropic::ior() const {
	return 1.5f;
}

void Sample_isotropic::sample(sampler::Sampler& sampler, bxdf::Result& result) const {
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

float3 Sample_anisotropic::evaluate(const float3& wi, float& pdf) const {
	if (!same_hemisphere(wo_)) {
		pdf = 0.f;
		return float3::identity();
	}

	const float n_dot_wi = layer_.clamp_n_dot(wi);
	const float n_dot_wo = layer_.clamp_abs_n_dot(wo_); //layer_.clamp_n_dot(wo_);

	const float3 h = math::normalize(wo_ + wi);
	const float wo_dot_h = clamp_dot(wo_, h);

	const fresnel::Conductor conductor(layer_.ior_, layer_.absorption_);
	return n_dot_wi * ggx::Anisotropic::reflection(h, n_dot_wi, n_dot_wo, wo_dot_h,
												   layer_, conductor, pdf);
}

float Sample_anisotropic::ior() const {
	return 1.5f;
}

void Sample_anisotropic::sample(sampler::Sampler& sampler, bxdf::Result& result) const {
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
