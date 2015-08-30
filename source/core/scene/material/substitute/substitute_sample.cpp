#include "substitute_sample.hpp"
#include "rendering/integrator/surface/integrator_helper.hpp"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/lambert/lambert.inl"
#include "scene/material/oren_nayar/oren_nayar.inl"
#include "sampler/sampler.hpp"
#include "base/math/vector.inl"
#include "base/math/math.hpp"
#include "base/math/sampling/sampling.inl"

// #include <iostream>

namespace scene { namespace material { namespace substitute {

Sample::Sample() : lambert_(*this), oren_nayar_(*this), ggx_(*this) {}

math::float3 Sample::evaluate(const math::float3& wi, float& pdf) const {
	// This is a bit complicated to understand:
	// If the material does not have transmission, we will never get a wi that is in the wrong hemisphere,
	// because that case is handled before coming here, so the check is only neccessary in that case.
	// On the other hand, if the there is transmission and wi is actullay coming from "behind", then we don't need
	// to calculate the reflection. In the other case, transmission won't be visible and we only need reflection.
	if (thickness_ > 0.f && math::dot(wi, geo_n_) < 0.f) {
		float n_dot_wi = std::max(-math::dot(n_, wi),  0.00001f);
		math::float3 attenuation = rendering::attenuation(thickness_, attenuation_);
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

	math::float3 diffuse = math::Pi_inv * (a + b * s * t) * diffuse_color_;

	// ----

	// Roughness zero will always have zero specular term (or worse NaN)
	if (0.f == a2_) {
		pdf = n_dot_wi * math::Pi_inv;
		return n_dot_wi * diffuse;
	}

	math::float3 h = math::normalized(wo_ + wi);

	float n_dot_h  = math::dot(n_, h);
	float wo_dot_h = math::dot(wo_, h);

	float d = ggx::d(n_dot_h, std::max(a2_, 0.0000001f));

	math::float3 specular = d * ggx::g(n_dot_wi, n_dot_wo, a2_) * ggx::f(wo_dot_h, f0_);

	float diffuse_pdf = n_dot_wi * math::Pi_inv;
	float ggx_pdf     = d * n_dot_h / (4.f * std::max(wo_dot_h, 0.00001f));

	pdf = 0.5f * (diffuse_pdf + ggx_pdf);

	if (thickness_ > 0.f) {
		pdf *= 0.5f;
	}

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

void Sample::sample_evaluate(sampler::Sampler& sampler, BxDF_result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	if (thickness_ > 0.f) {
		float p = sampler.generate_sample_1D();

		if (p < 0.5f) {
			float n_dot_wi = lambert_.importance_sample(sampler, result);
			result.wi *= -1.f;
			result.pdf *= 0.5f;
			math::float3 attenuation = rendering::attenuation(thickness_, attenuation_);
			result.reflection *= n_dot_wi * attenuation;
		} else {
			if (1.f == metallic_) {
				float n_dot_wi = ggx_.importance_sample(sampler, result);
				result.pdf *= 0.5f;
				result.reflection *= n_dot_wi;
			} else {
				float n_dot_wo = std::max(math::dot(n_, wo_), 0.00001f);

				if (p < 0.75f) {
					float n_dot_wi = oren_nayar_.importance_sample(sampler, n_dot_wo, result);
					result.pdf = 0.25f * (result.pdf + ggx_.pdf(result.wi, n_dot_wi));
					result.reflection = n_dot_wi * (result.reflection + ggx_.evaluate(result.wi, n_dot_wi, n_dot_wo));
				} else {
					float n_dot_wi = ggx_.importance_sample(sampler, result);
					result.pdf = 0.25f * (result.pdf + oren_nayar_.pdf(result.wi, n_dot_wi));
					result.reflection = n_dot_wi * (result.reflection +
													oren_nayar_.evaluate(result.wi, n_dot_wi, n_dot_wo));
				}
			}
		}
	} else {
		if (1.f == metallic_) {
			float n_dot_wi = ggx_.importance_sample(sampler, result);
			result.reflection *= n_dot_wi;
		} else {
			float p = sampler.generate_sample_1D();

			float n_dot_wo = std::max(math::dot(n_, wo_), 0.00001f);

			if (p < 0.5f) {
				float n_dot_wi = oren_nayar_.importance_sample(sampler, n_dot_wo, result);
				result.pdf = 0.5f * (result.pdf + ggx_.pdf(result.wi, n_dot_wi));
				result.reflection = n_dot_wi * (result.reflection + ggx_.evaluate(result.wi, n_dot_wi, n_dot_wo));
			} else {
				float n_dot_wi = ggx_.importance_sample(sampler, result);
				result.pdf = 0.5f * (result.pdf + oren_nayar_.pdf(result.wi, n_dot_wi));
				result.reflection = n_dot_wi * (result.reflection + oren_nayar_.evaluate(result.wi, n_dot_wi, n_dot_wo));
			}
		}
	}
}

bool Sample::is_pure_emissive() const {
	return false;
}

bool Sample::is_translucent() const {
	return thickness_ > 0.f;
}

void Sample::set(const math::float3& color, const math::float3& emission,
				 float sqrt_roughness, float metallic, float thickness, float attenuation_distance) {
	diffuse_color_ = (1.f - metallic) * color;
	f0_ = math::lerp(math::float3(0.03f, 0.03f, 0.03f), color, metallic);
	emission_ = emission;

	float roughness = sqrt_roughness * sqrt_roughness;
	a2_ = roughness * roughness;

	metallic_ = metallic;
	thickness_ = thickness;

	if (thickness > 0.f) {
		attenuation_ = math::float3(1.f / (diffuse_color_.x * attenuation_distance),
									1.f / (diffuse_color_.y * attenuation_distance),
									1.f / (diffuse_color_.z * attenuation_distance));
	}
}

}}}
