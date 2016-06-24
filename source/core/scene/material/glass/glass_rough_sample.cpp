/*
#include "glass_rough_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace glass {

float3 BRDF_rough::evaluate(const Sample_rough& sample,
							const float3& wi, float n_dot_wi) const {
	return math::float3_identity;
}

float BRDF_rough::pdf(const Sample_rough& sample, const float3& wi, float n_dot_wi) const {
	return 0.f;
}

float BRDF_rough::importance_sample(const Sample_rough& sample,
									sampler::Sampler& sampler, bxdf::Result& result) const {
	float3 n = sample.n_;
	float eta_i = 1.f / sample.ior_;
	float eta_t = sample.ior_;

	if (!sample.same_hemisphere(sample.wo_)) {
		n *= -1.f;
		eta_t = eta_i;
		eta_i = sample.ior_;
	}

	float n_dot_wo = math::saturate(math::dot(n, sample.wo_));

	result.wi = math::normalized(2.f * n_dot_wo * n - sample.wo_);

	float sint2 = (eta_i * eta_i) * (1.f - n_dot_wo * n_dot_wo);

	float f;
	if (sint2 > 1.f) {
		f = 1.f;
	} else {
		float n_dot_t = std::sqrt(1.f - sint2);

		// fresnel has to be the same value that would have been computed by BRDF
		f = fresnel::dielectric(n_dot_wo, n_dot_t, eta_i, eta_t);
	}

	result.reflection = float3(f);
	result.pdf = 1.f;
	result.type.clear_set(bxdf::Type::Specular_reflection);

	return 1.f;
}

float3 BTDF_rough::evaluate(const Sample_rough& sample,
								  const float3& wi, float n_dot_wi) const {
	return math::float3_identity;
}

float BTDF_rough::pdf(const Sample_rough& sample, const float3& wi, float n_dot_wi) const {
	return 0.f;
}

float BTDF_rough::importance_sample(const Sample_rough& sample,
									sampler::Sampler& sampler, bxdf::Result& result) const {
	float3 n = sample.n_;
	float eta_i = 1.f / sample.ior_;
	float eta_t = sample.ior_;

	if (!sample.same_hemisphere(sample.wo_)) {
		n *= -1.f;
		eta_t = eta_i;
		eta_i = sample.ior_;
	}

	float n_dot_wo = math::saturate(math::dot(n, sample.wo_));

	float sint2 = (eta_i * eta_i) * (1.f - n_dot_wo * n_dot_wo);

	if (sint2 > 1.f) {
		result.pdf = 0.f;
		return 0.f;
	}

	float n_dot_t = std::sqrt(1.f - sint2);
	result.wi = math::normalized((eta_i * n_dot_wo - n_dot_t) * n - eta_i * sample.wo_);

	// fresnel has to be the same value that would have been computed by BRDF
	float f = fresnel::dielectric(n_dot_wo, n_dot_t, eta_i, eta_t);

	result.reflection = (1.f - f) * sample.color_;
	result.pdf = 1.f;
	result.type.clear_set(bxdf::Type::Specular_transmission);

	return 1.f;
}


float3_p Sample_rough::shading_normal() const {
	return n;
}

float3 Sample_rough::tangent_to_world(float3_p v) const {
	return tangent_to_world(n, v);
}

float3 Sample_rough::evaluate(float3_p wi, float& pdf) const {
	pdf = 0.f;
	return math::float3_identity;
}

float3 Sample_rough::radiance() const {
	return math::float3_identity;
}

float3 Sample_rough::attenuation() const {
	return attenuation_;
}

float Sample_rough::ior() const {
	return ior_;
}

void Sample_rough::sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const {
	float p = sampler.generate_sample_1D();

	if (p < 0.5f) {
		brdf_.importance_sample(*this, sampler, result);
		result.pdf *= 0.5f;
	} else {
		btdf_.importance_sample(*this, sampler, result);
		result.pdf *= 0.5f;
	}

//	brdf_.importance_sample(sampler, result);

//	btdf_.importance_sample(sampler, result);

//	result.pdf *= 0.5f;
}

bool Sample_rough::is_pure_emissive() const {
	return false;
}

bool Sample_rough::is_transmissive() const {
	return true;
}

bool Sample_rough::is_translucent() const {
	return false;
}

void Sample_rough::set(float3_p n, float3_p color, float attenuation_distance,
					   float ior, float ior_outside) {
	this->n = n;
	color_ = color;
	attenuation_ = material::Sample::attenuation(color, attenuation_distance);
	ior_ = ior;
	ior_outside_ = ior_outside;
}

}}}
*/
