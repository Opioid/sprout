 #include "matte_sample.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/disney/disney.inl"
// #include "scene/material/lambert/lambert.inl"
// #include "scene/material/oren_nayar/oren_nayar.inl"
#include "base/math/vector4.inl"

namespace scene::material::matte {

const material::Sample::Layer& Sample::base_layer() const {
	return layer_;
}

bxdf::Result Sample::evaluate(const float3& wi) const {
	const float n_dot_wi = layer_.clamp_n_dot(wi);
	const float n_dot_wo = layer_.clamp_abs_n_dot(wo_); // layer_.clamp_n_dot(wo_);

//	float3 brdf = lambert::Isotropic::reflection(layer_.diffuse_color, n_dot_wi, layer_, pdf);

	float3 h = math::normalize(wo_ + wi);
	float h_dot_wi = clamp_dot(h, wi);

//	const float wi_dot_wo = math::dot(wi, wo_);
//	const float sl_wi_wo = 2.f + 2.f * wi_dot_wo;
//	const float rcpl_wi_wo = math::rsqrt(sl_wi_wo);
//	const float h_dot_wi = math::clamp(rcpl_wi_wo + rcpl_wi_wo * wi_dot_wo, 0.00001f, 1.f);

	const auto brdf = disney::Isotropic::reflection(h_dot_wi, n_dot_wi, n_dot_wo, layer_);

//	float3 brdf = oren_nayar::Isotropic::reflection(wi, wo_, n_dot_wi, n_dot_wo, layer_, pdf);

	return { n_dot_wi * brdf.reflection, brdf.pdf };
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Sample& result) const {
//	const float n_dot_wi = lambert::Isotropic::reflect(layer_.diffuse_color, layer_, sampler, result);

	const float n_dot_wo = layer_.clamp_abs_n_dot(wo_); // layer_.clamp_n_dot(wo_);

	const float n_dot_wi = disney::Isotropic::reflect(wo_, n_dot_wo, layer_, sampler, result);

	result.reflection *= n_dot_wi;
}

float Sample::ior() const {
	return 1.47f;
}

void Sample::Layer::set(const float3& color) {
	diffuse_color_ = color;
	roughness_ = 1.f;
	alpha2_ = math::pow4(roughness_);
}

}
