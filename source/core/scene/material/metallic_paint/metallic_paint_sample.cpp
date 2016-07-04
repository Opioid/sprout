#include "metallic_paint_sample.hpp"
#include "scene/material/coating/coating.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/lambert/lambert.inl"
#include "scene/material/material_sample.inl"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace metallic_paint {

float3_p Sample::shading_normal() const {
	return base_.n;
}

float3 Sample::tangent_to_world(float3_p v) const {
	return base_.tangent_to_world(v);
}

float3 Sample::evaluate(float3_p wi, float& pdf) const {
	if (!same_hemisphere(wo_)) {
		pdf = 0.f;
		return math::float3_identity;
	}

	float3 coating_attenuation;
	float  coating_pdf;
	float3 coating_reflection = coating_.evaluate(wi, wo_, 1.f, coating_attenuation, coating_pdf);

	float3 flakes_fresnel;
	float  flakes_pdf;
	float3 flakes_reflection = flakes_.evaluate(wi, wo_, flakes_fresnel, flakes_pdf);

	float  base_pdf;
	float3 base_reflection = (1.f - flakes_fresnel) * base_.evaluate(wi, wo_, base_pdf);

	pdf = (coating_pdf + flakes_pdf + base_pdf) / 3.f;

	return coating_reflection + coating_attenuation * (base_reflection + flakes_reflection);
}

float3 Sample::radiance() const {
	return math::float3_identity;
}

float3 Sample::attenuation() const {
	return float3(100.f, 100.f, 100.f);
}

float Sample::ior() const {
	return 1.5f;
}

void Sample::sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	float p = sampler.generate_sample_1D();

	if (p < 0.5f) {
		float3 coating_attenuation;
		coating_.importance_sample(wo_, 1.f, sampler, coating_attenuation, result);

		float3 flakes_fresnel;
		float  flakes_pdf;
		float3 flakes_reflection = flakes_.evaluate(result.wi, wo_,
													flakes_fresnel, flakes_pdf);

		float  base_pdf;
		float3 base_reflection = (1.f - flakes_fresnel) * base_.evaluate(result.wi, wo_, base_pdf);

		result.pdf = (result.pdf + base_pdf + flakes_pdf) / 3.f;
		result.reflection = result.reflection
						  + coating_attenuation * (base_reflection + flakes_reflection);
	} else {
		base_.importance_sample(wo_, sampler, result);

		float3 coating_attenuation;
		float  coating_pdf;
		float3 coating_reflection = coating_.evaluate(result.wi, wo_, 1.f,
													  coating_attenuation, coating_pdf);

		float3 flakes_fresnel;
		float  flakes_pdf;
		float3 flakes_reflection = flakes_.evaluate(result.wi, wo_,
													flakes_fresnel, flakes_pdf);

		float3 base_reflection = (1.f - flakes_fresnel) * result.reflection;

		result.pdf = (result.pdf + coating_pdf + flakes_pdf) / 3.f;
		result.reflection = coating_reflection
						  + coating_attenuation * (base_reflection + flakes_reflection);
	}
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

void Sample::Base_layer::set(float3_p color_a, float3_p color_b) {
	this->color_a = color_a;
	this->color_b = color_b;

	this->a2 = 0.1f;
}

float3 Sample::Base_layer::evaluate(float3_p wi, float3_p wo, float& pdf) const {
	float n_dot_wi = clamped_n_dot(wi);
	float n_dot_wo = clamped_n_dot(wo);

	float f = n_dot_wo;

	float3 color = math::lerp(color_b, color_a, f);

	fresnel::Schlick fresnel(color);

	float3 ggx_reflection = ggx::Isotropic::evaluate(wi, wo, n_dot_wi, n_dot_wo,
													 *this, fresnel, pdf);

	return n_dot_wi * ggx_reflection;
}

void Sample::Base_layer::importance_sample(float3_p wo, sampler::Sampler& sampler,
										   bxdf::Result& result) const {
	float n_dot_wo = clamped_n_dot(wo);

	float f = n_dot_wo;

	float3 color = math::lerp(color_b, color_a, f);

	fresnel::Schlick fresnel(color);

	float n_dot_wi = ggx::Isotropic::importance_sample(wo, n_dot_wo, *this,
													   fresnel, sampler, result);
	result.reflection *= n_dot_wi;
}

float3 Sample::Flakes_layer::evaluate(float3_p wi, float3_p wo,
									  float3& fresnel_result, float& pdf) const {
	float n_dot_wi = clamped_n_dot(wi);
	float n_dot_wo = clamped_n_dot(wo);

	fresnel::Conductor conductor(ior, absorption);
	return n_dot_wi * ggx::Isotropic::evaluate(wi, wo, n_dot_wi, n_dot_wo,
											   *this, conductor, fresnel_result, pdf);
}

void Sample::Flakes_layer::importance_sample(float3_p wo, sampler::Sampler& sampler,
											 float3& fresnel_result, bxdf::Result& result) const {
	float n_dot_wo = clamped_n_dot(wo);

	fresnel_result = float3(fresnel::schlick(n_dot_wo, 1.f));

	fresnel::Conductor conductor(ior, absorption);
	float n_dot_wi = ggx::Isotropic::importance_sample(wo, n_dot_wo, *this,
													   conductor, sampler, fresnel_result, result);
	result.reflection *= n_dot_wi;
}

}}}
