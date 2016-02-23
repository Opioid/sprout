#include "substitute_sample.hpp"
#include "rendering/integrator/surface/integrator_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/lambert/lambert.inl"
#include "scene/material/oren_nayar/oren_nayar.inl"
#include "sampler/sampler.hpp"
#include "base/math/vector.inl"
#include "base/math/math.hpp"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace material { namespace substitute {

math::vec3 Sample::evaluate(math::pvec3 wi, float& pdf) const {
	if (!same_hemisphere(wo_)) {
		pdf = 0.f;
		return math::vec3_identity;
	}

	// This is a bit complicated to understand:
	// If the material does not have transmission, we will never get a wi which is in the wrong hemisphere,
	// because that case is handled before coming here,
	// so the check is only neccessary transmissive materials (codified by thickness > 0).
	// On the other hand, if the there is transmission and wi is actullay coming from "behind", then we don't need
	// to calculate the reflection. In the other case, transmission won't be visible and we only need reflection.
	if (thickness_ > 0.f && !same_hemisphere(wi)) {
		float n_dot_wi = std::max(-math::dot(n_, wi),  0.00001f);
		float approximated_distance = thickness_ / n_dot_wi;
		math::vec3 attenuation = rendering::attenuation(approximated_distance, attenuation_);
		pdf = 0.5f * n_dot_wi * math::Pi_inv;
		return n_dot_wi * (math::Pi_inv * attenuation * diffuse_color_);
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

	math::vec3 diffuse = math::Pi_inv * (a + b * s * t) * diffuse_color_;
	float diffuse_pdf = n_dot_wi * math::Pi_inv;
	// ----

	// Roughness zero will always have zero specular term (or worse NaN)
	if (0.f == a2_) {
		pdf = diffuse_pdf;
		return n_dot_wi * diffuse;
	}

	math::vec3 h = math::normalized(wo_ + wi);

	float n_dot_h  = math::dot(n_, h);
	float wo_dot_h = math::dot(wo_, h);

	float clamped_a2 = ggx::clamp_a2(a2_);
	float d = ggx::distribution_isotropic(n_dot_h, clamped_a2);
	float g = ggx::geometric_shadowing(n_dot_wi, n_dot_wo, clamped_a2);
	math::vec3 f = fresnel::schlick(wo_dot_h, f0_);

	math::vec3 specular = d * g * f;

	// this helped in the past, but problem maybe caused by faulty sphere normals
//	float ggx_pdf     = d * n_dot_h / (4.f * std::max(wo_dot_h, 0.00001f));
	float ggx_pdf     = d * n_dot_h / (4.f * wo_dot_h);

	pdf = 0.5f * (diffuse_pdf + ggx_pdf);

	if (thickness_ > 0.f) {
		pdf *= 0.5f;
	}

//	if (math::contains_negative(diffuse) || math::contains_negative(specular)) {
//		std::cout << "substitute::Sample::evaluate()" << std::endl;
//	}

	return n_dot_wi * (diffuse + specular);
}

math::vec3 Sample::emission() const {
	return emission_;
}

math::vec3 Sample::attenuation() const {
	return math::vec3(100.f, 100.f, 100.f);
}

float Sample::ior() const {
	return 0.f;
}

void Sample::sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	if (thickness_ > 0.f) {
		float p = sampler.generate_sample_1D();

		if (p < 0.5f) {
			float n_dot_wi = lambert_.importance_sample(*this, sampler, result);
			result.wi *= -1.f;
			result.pdf *= 0.5f;
			float approximated_distance = thickness_ / n_dot_wi;
			math::vec3 attenuation = rendering::attenuation(approximated_distance, attenuation_);
			result.reflection *= n_dot_wi * attenuation;
		} else {
			if (1.f == metallic_) {
				float n_dot_wo = clamped_n_dot_wo();
				float n_dot_wi = ggx_.importance_sample(*this, sampler, n_dot_wo, result);
				result.reflection *= n_dot_wi;
				result.pdf *= 0.5f;
			} else {
				float n_dot_wo = clamped_n_dot_wo();

				if (p < 0.75f) {
					float n_dot_wi = oren_nayar_.importance_sample(*this, sampler, n_dot_wo, result);

					float ggx_pdf;
					math::vec3 ggx_reflection = ggx_.evaluate(*this, result.wi, n_dot_wi, n_dot_wo, ggx_pdf);

					result.reflection = n_dot_wi * (result.reflection + ggx_reflection);
					result.pdf = 0.25f * (result.pdf + ggx_pdf);

				} else {
					float n_dot_wi = ggx_.importance_sample(*this, sampler, n_dot_wo, result);

					float oren_nayar_pdf;
					math::vec3 oren_nayar_reflection = oren_nayar_.evaluate(*this, result.wi, n_dot_wi, n_dot_wo,
																			  oren_nayar_pdf);

					result.reflection = n_dot_wi * (result.reflection + oren_nayar_reflection);
					result.pdf = 0.25f * (result.pdf + oren_nayar_pdf);
				}
			}
		}
	} else {
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
				math::vec3 ggx_reflection = ggx_.evaluate(*this, result.wi, n_dot_wi, n_dot_wo, ggx_pdf);

				result.reflection = n_dot_wi * (result.reflection + ggx_reflection);
				result.pdf = 0.5f * (result.pdf + ggx_pdf);
			} else {
				float n_dot_wi = ggx_.importance_sample(*this, sampler, n_dot_wo, result);

				float oren_nayar_pdf;
				math::vec3 oren_nayar_reflection = oren_nayar_.evaluate(*this, result.wi, n_dot_wi, n_dot_wo,
																		  oren_nayar_pdf);

				result.reflection = n_dot_wi * (result.reflection + oren_nayar_reflection);
				result.pdf = 0.5f * (result.pdf + oren_nayar_pdf);
			}
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
	return thickness_ > 0.f;
}

void Sample::set(const math::vec3& color, const math::vec3& emission,
				 float constant_f0, float roughness, float metallic,
				 float thickness, float attenuation_distance) {
	diffuse_color_ = (1.f - metallic) * color;
	f0_ = math::lerp(math::vec3(constant_f0), color, metallic);
	emission_ = emission;

	float a = roughness * roughness;
	a2_ = a * a;

	metallic_ = metallic;
	thickness_ = thickness;

	if (thickness > 0.f) {
		attenuation_ = material::Sample::attenuation(diffuse_color_, attenuation_distance);
	}
}

}}}
