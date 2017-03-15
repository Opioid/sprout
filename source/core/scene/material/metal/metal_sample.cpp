#include "metal_sample.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/ggx/ggx.inl"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"

namespace scene { namespace material { namespace metal {

const material::Sample::Layer& Sample_isotropic::base_layer() const {
	return layer_;
}

float3 Sample_isotropic::evaluate(float3_p wi, float& pdf) const {
	if (!same_hemisphere(wo_)) {
		pdf = 0.f;
		return float3::identity();
	}

	float n_dot_wi = layer_.clamped_n_dot(wi);
	float n_dot_wo = layer_.clamped_n_dot(wo_);

	float3 h = math::normalized(wo_ + wi);
	float wo_dot_h = math::clamp(math::dot(wo_, h), 0.00001f, 1.f);

	const float n_dot_h = math::saturate(math::dot(layer_.n_, h));

	fresnel::Conductor conductor(layer_.ior_, layer_.absorption_);
	return n_dot_wi * ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h,
												 layer_, conductor, pdf);
}

float3 Sample_isotropic::radiance() const {
	return float3::identity();
}

float3 Sample_isotropic::attenuation() const {
	return float3(100.f, 100.f, 100.f);
}

float Sample_isotropic::ior() const {
	return 1.5f;
}

void Sample_isotropic::sample(sampler::Sampler& sampler, bxdf::Result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	float n_dot_wo = layer_.clamped_n_dot(wo_);

	fresnel::Conductor conductor(layer_.ior_, layer_.absorption_);
	float n_dot_wi = ggx::Isotropic::reflect(wo_, n_dot_wo, layer_, conductor,
											 sampler, result);
	result.reflection *= n_dot_wi;
}

bool Sample_isotropic::is_pure_emissive() const {
	return false;
}

bool Sample_isotropic::is_transmissive() const {
	return false;
}

bool Sample_isotropic::is_translucent() const {
	return false;
}

void Sample_isotropic::Layer::set(float3_p ior, float3_p absorption, float roughness) {
	ior_ = ior;
	absorption_ = absorption;
	a2_ = math::pow4(roughness);
}

const material::Sample::Layer& Sample_anisotropic::base_layer() const {
	return layer_;
}

float3 Sample_anisotropic::evaluate(float3_p wi, float& pdf) const {
	if (!same_hemisphere(wo_)) {
		pdf = 0.f;
		return float3::identity();
	}

	float n_dot_wi = layer_.clamped_n_dot(wi);
	float n_dot_wo = layer_.clamped_n_dot(wo_);

	float3 h = math::normalized(wo_ + wi);
	float wo_dot_h = math::clamp(math::dot(wo_, h), 0.00001f, 1.f);

	fresnel::Conductor conductor(layer_.ior_, layer_.absorption_);
	return n_dot_wi * ggx::Anisotropic::reflection(h, n_dot_wi, n_dot_wo, wo_dot_h,
												   layer_, conductor, pdf);
}

float3 Sample_anisotropic::radiance() const {
	return float3::identity();
}

float3 Sample_anisotropic::attenuation() const {
	return float3(100.f, 100.f, 100.f);
}

float Sample_anisotropic::ior() const {
	return 1.5f;
}

void Sample_anisotropic::sample(sampler::Sampler& sampler, bxdf::Result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	float n_dot_wo = layer_.clamped_n_dot(wo_);

	fresnel::Conductor conductor(layer_.ior_, layer_.absorption_);
	float n_dot_wi = ggx::Anisotropic::reflect(wo_, n_dot_wo, layer_, conductor,
											   sampler, result);
	result.reflection *= n_dot_wi;
}

bool Sample_anisotropic::is_pure_emissive() const {
	return false;
}

bool Sample_anisotropic::is_transmissive() const {
	return false;
}

bool Sample_anisotropic::is_translucent() const {
	return false;
}

void Sample_anisotropic::Layer::set(float3_p ior, float3_p absorption, float2 roughness) {
	ior_ = ior;
	absorption_ = absorption;

	float2 a = roughness * roughness;
	a_   = a;
	a2_  = a * a;
	axy_ = a[0] * a[1];
}

}}}
