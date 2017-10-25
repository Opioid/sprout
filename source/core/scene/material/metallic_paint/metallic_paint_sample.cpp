#include "metallic_paint_sample.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/coating/coating.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_sample.inl"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"

namespace scene::material::metallic_paint {

const material::Sample::Layer& Sample::base_layer() const {
	return base_;
}

bxdf::Result Sample::evaluate(const float3& wi) const {
	if (!same_hemisphere(wo_)) {
		return { float3::identity(), 0.f };
	}

	float3 h = math::normalize(wo_ + wi);
	float wo_dot_h = clamp_dot(wo_, h);

	float3 coating_attenuation;
	const auto coating = coating_.evaluate(wi, wo_, h, wo_dot_h, 1.f, coating_attenuation);

	float3 flakes_fresnel;
	const auto flakes = flakes_.evaluate(wi, wo_, h, wo_dot_h, flakes_fresnel);

	const auto base = base_.evaluate(wi, wo_, h, wo_dot_h);

	const float3 bottom = ((1.f - flakes_fresnel) * base.reflection + flakes.reflection);

	const float pdf = (coating.pdf + flakes.pdf + base.pdf) / 3.f;

	return { coating.reflection + coating_attenuation * bottom, pdf };
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Sample& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	float p = sampler.generate_sample_1D();

	if (p < 0.4f) {
		float3 coating_attenuation;
		coating_.sample(wo_, 1.f, sampler, coating_attenuation, result);

		float3 flakes_fresnel;
		const auto flakes = flakes_.evaluate(result.wi, wo_, result.h,
											 result.h_dot_wi, flakes_fresnel);

		const auto base = base_.evaluate(result.wi, wo_, result.h, result.h_dot_wi);

		const float3 bottom = ((1.f - flakes_fresnel) * base.reflection + flakes.reflection);

		result.reflection = result.reflection + coating_attenuation * bottom;
		result.pdf = (result.pdf + base.pdf + flakes.pdf) / 3.f;
	} else if (p < 0.7f) {
		base_.sample(wo_, sampler, result);

		float3 coating_attenuation;
		const auto coating = coating_.evaluate(result.wi, wo_, result.h, result.h_dot_wi,
											   1.f, coating_attenuation);

		float3 flakes_fresnel;
		const auto flakes = flakes_.evaluate(result.wi, wo_, result.h,
											 result.h_dot_wi, flakes_fresnel);

		const float3 bottom = (1.f - flakes_fresnel) * result.reflection + flakes.reflection;

		result.reflection = coating.reflection + coating_attenuation * bottom;
		result.pdf = (result.pdf + coating.pdf + flakes.pdf) / 3.f;
	} else {
		float3 flakes_fresnel;
		flakes_.sample(wo_, sampler, flakes_fresnel, result);

		float3 coating_attenuation;
		const auto coating = coating_.evaluate(result.wi, wo_, result.h, result.h_dot_wi,
											   1.f, coating_attenuation);

		const auto base = base_.evaluate(result.wi, wo_, result.h, result.h_dot_wi);

		const float3 bottom = ((1.f - flakes_fresnel) * base.reflection + result.reflection);

		result.reflection = coating.reflection + coating_attenuation * bottom;
		result.pdf = (result.pdf + base.pdf + coating.pdf) / 3.f;
	}
}

float Sample::ior() const {
	return 1.5f;
}

void Sample::Base_layer::set(const float3& color_a, const float3& color_b,
							 float alpha, float alpha2) {
	color_a_ = color_a;
	color_b_ = color_b;
	alpha_  = alpha;
	alpha2_ = alpha2;
}

bxdf::Result Sample::Base_layer::evaluate(const float3& wi, const float3& wo, const float3& h,
									float wo_dot_h) const {
	float n_dot_wi = clamp_n_dot(wi);
	float n_dot_wo = clamp_abs_n_dot(wo); //clamp_n_dot(wo);

	float f = n_dot_wo;

	float3 color = math::lerp(color_b_, color_a_, f);

	const float n_dot_h = math::saturate(math::dot(n_, h));

	fresnel::Schlick fresnel(color);
	const auto ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h,
												*this, fresnel);

	return { n_dot_wi * ggx.reflection, ggx.pdf };
}

void Sample::Base_layer::sample(const float3& wo, sampler::Sampler& sampler,
								bxdf::Sample& result) const {
	float n_dot_wo = clamp_abs_n_dot(wo); //clamp_n_dot(wo);

	float f = n_dot_wo;

	float3 color = math::lerp(color_b_, color_a_, f);

	fresnel::Schlick fresnel(color);
	float n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, *this, fresnel, sampler, result);
	result.reflection *= n_dot_wi;
}

void Sample::Flakes_layer::set(const float3& ior, const float3& absorption,
							   float alpha, float alpha2, float weight) {
	ior_ = ior;
	absorption_ = absorption;
	alpha_  = alpha;
	alpha2_ = alpha2;
	weight_ = weight;
}

bxdf::Result Sample::Flakes_layer::evaluate(const float3& wi, const float3& wo, const float3& h,
											float wo_dot_h, float3& fresnel_result) const {
	float n_dot_wi = clamp_n_dot(wi);
	float n_dot_wo = clamp_abs_n_dot(wo); //clamp_n_dot(wo);

	const float n_dot_h = math::saturate(math::dot(n_, h));

	fresnel::Conductor_weighted conductor(ior_, absorption_, weight_);
	const auto ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h,
												*this, conductor, fresnel_result);

	return { n_dot_wi * ggx.reflection, ggx.pdf };
}

void Sample::Flakes_layer::sample(const float3& wo, sampler::Sampler& sampler,
								  float3& fresnel_result, bxdf::Sample& result) const {
	float n_dot_wo = clamp_abs_n_dot(wo); //clamp_n_dot(wo);

	fresnel::Conductor_weighted conductor(ior_, absorption_, weight_);
	float n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, *this, conductor, sampler,
											 fresnel_result, result);
	result.reflection *= n_dot_wi;
}

}
