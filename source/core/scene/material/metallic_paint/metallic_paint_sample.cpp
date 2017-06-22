#include "metallic_paint_sample.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/coating/coating.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_sample.inl"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"

namespace scene { namespace material { namespace metallic_paint {

const material::Sample::Layer& Sample::base_layer() const {
	return base_;
}

float3 Sample::evaluate(const float3& wi, float& pdf) const {
	if (!same_hemisphere(wo_)) {
		pdf = 0.f;
		return float3::identity();
	}

	float3 h = math::normalized(wo_ + wi);
	float wo_dot_h = clamped_dot(wo_, h);

	float3 coating_attenuation;
	float  coating_pdf;
	float3 coating_reflection = coating_.evaluate(wi, wo_, h, wo_dot_h, 1.f,
												  coating_attenuation, coating_pdf);

	float3 flakes_fresnel;
	float  flakes_pdf;
	float3 flakes_reflection = flakes_.evaluate(wi, wo_, h, wo_dot_h,
												flakes_fresnel, flakes_pdf);

	float  base_pdf;
	float3 base_reflection = (1.f - flakes_fresnel) * base_.evaluate(wi, wo_, h,
																	 wo_dot_h, base_pdf);

	pdf = (coating_pdf + flakes_pdf + base_pdf) / 3.f;

	return coating_reflection + coating_attenuation * (base_reflection + flakes_reflection);
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	float p = sampler.generate_sample_1D();

	if (p < 0.4f) {
		float3 coating_attenuation;
		coating_.sample(wo_, 1.f, sampler, coating_attenuation, result);

		float3 flakes_fresnel;
		float  flakes_pdf;
		float3 flakes_reflection = flakes_.evaluate(result.wi, wo_, result.h,
													result.h_dot_wi, flakes_fresnel, flakes_pdf);

		float  base_pdf;
		float3 base_reflection = (1.f - flakes_fresnel) * base_.evaluate(result.wi, wo_, result.h,
																		 result.h_dot_wi, base_pdf);

		result.pdf = (result.pdf + base_pdf + flakes_pdf) / 3.f;
		result.reflection = result.reflection
						  + coating_attenuation * (base_reflection + flakes_reflection);
	} else if (p < 0.7f) {
		base_.sample(wo_, sampler, result);

		float3 coating_attenuation;
		float  coating_pdf;
		float3 coating_reflection = coating_.evaluate(result.wi, wo_, result.h, result.h_dot_wi,
													  1.f, coating_attenuation, coating_pdf);

		float3 flakes_fresnel;
		float  flakes_pdf;
		float3 flakes_reflection = flakes_.evaluate(result.wi, wo_, result.h,
													result.h_dot_wi, flakes_fresnel, flakes_pdf);

		float3 base_reflection = (1.f - flakes_fresnel) * result.reflection;

		result.pdf = (result.pdf + coating_pdf + flakes_pdf) / 3.f;
		result.reflection = coating_reflection
						  + coating_attenuation * (base_reflection + flakes_reflection);
	} else {
		float3 flakes_fresnel;
		flakes_.sample(wo_, sampler, flakes_fresnel, result);

		float3 coating_attenuation;
		float  coating_pdf;
		float3 coating_reflection = coating_.evaluate(result.wi, wo_, result.h, result.h_dot_wi,
													  1.f, coating_attenuation, coating_pdf);

		float  base_pdf;
		float3 base_reflection = (1.f - flakes_fresnel) * base_.evaluate(result.wi, wo_, result.h,
																		 result.h_dot_wi, base_pdf);

		result.pdf = (result.pdf + base_pdf + coating_pdf) / 3.f;
		result.reflection = coating_reflection
						  + coating_attenuation * (base_reflection + result.reflection);
	}
}

float3 Sample::radiance() const {
	return float3::identity();
}

float3 Sample::attenuation() const {
	return float3(100.f, 100.f, 100.f);
}

float Sample::ior() const {
	return 1.5f;
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

void Sample::Base_layer::set(const float3& color_a, const float3& color_b, float a2) {
	color_a_ = color_a;
	color_b_ = color_b;
	a2_ = a2;
}

float3 Sample::Base_layer::evaluate(const float3& wi, const float3& wo, const float3& h,
									float wo_dot_h, float& pdf) const {
	float n_dot_wi = clamped_n_dot(wi);
	float n_dot_wo = clamped_n_dot(wo);

	float f = n_dot_wo;

	float3 color = math::lerp(color_b_, color_a_, f);

	const float n_dot_h = math::saturate(math::dot(n_, h));

	fresnel::Schlick fresnel(color);
	float3 ggx_reflection = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h,
													   *this, fresnel, pdf);

	return n_dot_wi * ggx_reflection;
}

void Sample::Base_layer::sample(const float3& wo, sampler::Sampler& sampler,
								bxdf::Result& result) const {
	float n_dot_wo = clamped_n_dot(wo);

	float f = n_dot_wo;

	float3 color = math::lerp(color_b_, color_a_, f);

	fresnel::Schlick fresnel(color);
	float n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, *this, fresnel,
											 sampler, result);
	result.reflection *= n_dot_wi;
}

void Sample::Flakes_layer::set(const float3& ior, const float3& absorption, float a2, float weight) {
	ior_ = ior;
	absorption_ = absorption;
	a2_ = a2;
	weight_ = weight;
}

float3 Sample::Flakes_layer::evaluate(const float3& wi, const float3& wo, const float3& h, float wo_dot_h,
									  float3& fresnel_result, float& pdf) const {
	float n_dot_wi = clamped_n_dot(wi);
	float n_dot_wo = clamped_n_dot(wo);

	const float n_dot_h = math::saturate(math::dot(n_, h));

	fresnel::Conductor_weighted conductor(ior_, absorption_, weight_);
	return n_dot_wi * ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h,
												 *this, conductor, fresnel_result, pdf);
}

void Sample::Flakes_layer::sample(const float3& wo, sampler::Sampler& sampler,
								  float3& fresnel_result, bxdf::Result& result) const {
	float n_dot_wo = clamped_n_dot(wo);

	fresnel::Conductor_weighted conductor(ior_, absorption_, weight_);
	float n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, *this, conductor, sampler,
											 fresnel_result, result);
	result.reflection *= n_dot_wi;
}

}}}
