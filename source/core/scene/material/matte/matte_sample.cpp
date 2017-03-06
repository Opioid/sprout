#include "matte_sample.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/disney/disney.inl"
// #include "scene/material/lambert/lambert.inl"
// #include "scene/material/oren_nayar/oren_nayar.inl"
#include "base/math/vector4.inl"

namespace scene { namespace material { namespace matte {

const material::Sample::Layer& Sample::base_layer() const {
	return layer_;
}

float3 Sample::evaluate(float3_p wi, float& pdf) const {
	float n_dot_wi = layer_.clamped_n_dot(wi);

//	float3 brdf = lambert::Isotropic::reflection(layer_.diffuse_color, n_dot_wi, layer_, pdf);

	float3 h = math::normalized(wo_ + wi);
	float h_dot_wi = math::clamp(math::dot(h, wi), 0.00001f, 1.f);

	float n_dot_wo = layer_.clamped_n_dot(wo_);

	float3 brdf = disney::Isotropic::reflection(h_dot_wi, n_dot_wi, n_dot_wo, layer_, pdf);

//	float3 brdf = oren_nayar::Isotropic::reflection(wi, wo_, n_dot_wi, n_dot_wo, layer_, pdf);

	return n_dot_wi * brdf;
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Result& result) const {
//	float n_dot_wi = lambert::Isotropic::reflect(layer_.diffuse_color, layer_, sampler, result);

	float n_dot_wo = layer_.clamped_n_dot(wo_);

	float n_dot_wi = disney::Isotropic::reflect(wo_, n_dot_wo, layer_, sampler, result);

	result.reflection *= n_dot_wi;
}

float3 Sample::radiance() const {
	return float3::identity();
}

float3 Sample::attenuation() const {
	return float3(100.f, 100.f, 100.f);
}

float Sample::ior() const {
	return 1.47f;
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

void Sample::Layer::set(float3_p color) {
	diffuse_color_ = color;
	roughness_ = 1.f;
	a2_ = math::pow4(roughness_);
}

}}}
