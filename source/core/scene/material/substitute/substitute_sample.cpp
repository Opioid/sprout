#include "substitute_sample.hpp"
#include "rendering/integrator/surface/integrator_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/oren_nayar/oren_nayar.inl"
#include "sampler/sampler.hpp"
#include "base/math/vector.inl"
#include "base/math/math.hpp"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace material { namespace substitute {

math::float3 Sample::evaluate(math::pfloat3 wi, float& pdf) const {
	if (!same_hemisphere(wo_)) {
		pdf = 0.f;
		return math::float3_identity;
	}

	float n_dot_wi = std::max(math::dot(n_, wi),  0.00001f);
	float n_dot_wo = std::max(math::dot(n_, wo_), 0.00001f);

	// oren nayar

	float wi_dot_wo = math::dot(wi, wo_);

	float s = wi_dot_wo - n_dot_wi * n_dot_wo;

	float t;
	if (s >= 0.f) {
		t = std::min(1.f, n_dot_wi / n_dot_wo);
	} else {
		t = n_dot_wi;
	}

	float a2 = a2_;
	float a = 1.f - 0.5f * (a2 / (a2 + 0.33f));
	float b = 0.45f * (a2 / (a2 + 0.09f));

	math::float3 diffuse = math::Pi_inv * (a + b * s * t) * diffuse_color_;
	float diffuse_pdf = n_dot_wi * math::Pi_inv;
	// ----

	// Roughness zero will always have zero specular term (or worse NaN)
	if (0.f == a2_) {
		pdf = diffuse_pdf;
		return n_dot_wi * diffuse;
	}

	math::float3 h = math::normalized(wo_ + wi);

	float n_dot_h  = math::dot(n_, h);
	float wo_dot_h = math::dot(wo_, h);

	float clamped_a2 = ggx::clamp_a2(a2_);
	float d = ggx::distribution_isotropic(n_dot_h, clamped_a2);
	float g = ggx::geometric_shadowing(n_dot_wi, n_dot_wo, clamped_a2);
	math::float3 f = fresnel::schlick(wo_dot_h, f0_);

	math::float3 specular = d * g * f;

	// this helped in the past, but problem maybe caused by faulty sphere normals
//	float ggx_pdf     = d * n_dot_h / (4.f * std::max(wo_dot_h, 0.00001f));
	float ggx_pdf     = d * n_dot_h / (4.f * wo_dot_h);

	pdf = 0.5f * (diffuse_pdf + ggx_pdf);

//	if (math::contains_negative(diffuse) || math::contains_negative(specular)) {
//		std::cout << "substitute::Sample::evaluate()" << std::endl;
//	}

	return n_dot_wi * (diffuse + specular);
}

math::float3 Sample::emission() const {
	return emission_;
}

math::float3 Sample::attenuation() const {
	return math::float3(100.f, 100.f, 100.f);
}

float Sample::ior() const {
	return 0.f;
}

void Sample::sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	if (1.f == metallic_) {
		float n_dot_wo = clamped_n_dot_wo();
		float n_dot_wi = ggx_.importance_sample(*this, sampler, n_dot_wo, result);
		result.reflection *= n_dot_wi;
	} else {
		float p = sampler.generate_sample_1D();

		float n_dot_wo = clamped_n_dot_wo();

		if (p < 0.5f) {
			float n_dot_wi = oren_nayar_.importance_sample(*this, sampler, n_dot_wo, result);

			float ggx_pdf;
			math::float3 ggx_reflection = ggx_.evaluate(*this, result.wi, n_dot_wi, n_dot_wo, ggx_pdf);

			result.reflection = n_dot_wi * (result.reflection + ggx_reflection);
			result.pdf = 0.5f * (result.pdf + ggx_pdf);
		} else {
			float n_dot_wi = ggx_.importance_sample(*this, sampler, n_dot_wo, result);

			float oren_nayar_pdf;
			math::float3 oren_nayar_reflection = oren_nayar_.evaluate(*this, result.wi, n_dot_wi, n_dot_wo,
																		  oren_nayar_pdf);

			result.reflection = n_dot_wi * (result.reflection + oren_nayar_reflection);
			result.pdf = 0.5f * (result.pdf + oren_nayar_pdf);
		}
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

void Sample::set(const math::float3& color, const math::float3& emission,
				 float constant_f0, float roughness, float metallic) {
	diffuse_color_ = (1.f - metallic) * color;
	f0_ = math::lerp(math::float3(constant_f0), color, metallic);
	emission_ = emission;

	float a = roughness * roughness;
	a2_ = a * a;

	metallic_ = metallic;
}

}}}
