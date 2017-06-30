#include "display_sample.hpp"
#include "scene/material/material_sample.inl"
#include "sampler/sampler.hpp"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "base/math/vector3.inl"

namespace scene { namespace material { namespace display {

const material::Sample::Layer& Sample::base_layer() const {
	return layer_;
}

float3 Sample::evaluate(const float3& wi, float& pdf) const {
	if (!same_hemisphere(wo_)) {
		pdf = 0.f;
		return float3::identity();
	}

	float n_dot_wi = layer_.clamped_n_dot(wi);
	float n_dot_wo = layer_.clamped_n_dot(wo_);

	float3 h = math::normalized(wo_ + wi);
	float wo_dot_h = clamped_dot(wo_, h);

	const float n_dot_h = math::saturate(math::dot(layer_.n_, h));

	fresnel::Schlick schlick(layer_.f0_);
	float3 ggx_reflection = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h,
													   layer_, schlick, pdf);

	return n_dot_wi * ggx_reflection;
}

float3 Sample::radiance() const {
	return layer_.emission_;
}

float3 Sample::absorption_coffecient() const {
	return float3(100.f, 100.f, 100.f);
}

float Sample::ior() const {
	return 1.5f;
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	float n_dot_wo = layer_.clamped_n_dot(wo_);
	fresnel::Schlick schlick(layer_.f0_);
	float n_dot_wi = ggx::Isotropic::reflect(wo_, n_dot_wo, layer_,
											 schlick, sampler, result);

	result.reflection *= n_dot_wi;
}

bool Sample::is_pure_emissive() const {
	return false;
}

bool Sample::is_transmissive() const {
	return false;
}

bool Sample::is_translucent() const {
	return false;
}

void Sample::Layer::set(const float3& radiance, float f0, float roughness) {
	emission_ = radiance;
	f0_ = float3(f0);
	a2_ = math::pow4(roughness);
}

}}}
