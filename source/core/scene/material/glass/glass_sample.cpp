#include "glass_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace glass {

// Adapted from https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/#more-1921
float fresnel_dielectric(float n_dot_wi, float n_dot_wo, float eta) {
	float t1 = eta * n_dot_wo;
	float t2 = eta * n_dot_wi;

	float rs = (n_dot_wi + t1) / (n_dot_wi - t1);
	float rp = (n_dot_wo + t2) / (n_dot_wo - t2);

	return 0.5f * (rs * rs + rp * rp);
}

float fresnel_dielectric_holgerusan(float cos_theta_i, float cos_theta_t, float eta_i, float eta_t) {
	float r_p = (eta_t * cos_theta_i + eta_i * cos_theta_t) / (eta_t * cos_theta_i - eta_i * cos_theta_t);
	float r_o = (eta_i * cos_theta_i + eta_t * cos_theta_t) / (eta_i * cos_theta_i - eta_t * cos_theta_t);

	return 0.5f * (r_p * r_p + r_o * r_o);
}

math::float3 BRDF::evaluate(const Sample& /*sample*/, const math::float3& /*wi*/, float /*n_dot_wi*/) const {
	return math::float3::identity;
}

float BRDF::pdf(const Sample& /*sample*/, const math::float3& /*wi*/, float /*n_dot_wi*/) const {
	return 0.f;
}

float BRDF::importance_sample(const Sample& sample, sampler::Sampler& /*sampler*/, bxdf::Result& result) const {
	math::float3 n = sample.n_;
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
		float n_dot_t = -std::sqrt(1.f - sint2);

		// fresnel has to be the same value that would have been computed by BRDF

	//	f = fresnel_dielectric(n_dot_t, n_dot_wo, eta);

		f = fresnel_dielectric_holgerusan(n_dot_wo, n_dot_t, eta_i, eta_t);
	}

	result.reflection = math::float3(f);
	result.pdf = 1.f;
	result.type.clear_set(bxdf::Type::Specular_reflection);

	return 1.f;
}

math::float3 BTDF::evaluate(const Sample& /*sample*/, const math::float3& /*wi*/, float /*n_dot_wi*/) const {
	return math::float3::identity;
}

float BTDF::pdf(const Sample& /*sample*/, const math::float3& /*wi*/, float /*n_dot_wi*/) const {
	return 0.f;
}

float BTDF::importance_sample(const Sample& sample, sampler::Sampler& /*sampler*/, bxdf::Result& result) const {
	math::float3 n = sample.n_;
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

	float n_dot_t = -std::sqrt(1.f - sint2);
	result.wi = math::normalized((eta_i * n_dot_wo + n_dot_t) * n - eta_i * sample.wo_);

	// fresnel has to be the same value that would have been computed by BRDF
//	float f = fresnel_dielectric(n_dot_t, n_dot_wo, eta);

	float f = fresnel_dielectric_holgerusan(n_dot_wo, n_dot_t, eta_i, eta_t);

	result.reflection = (1.f - f) * sample.color_;
	result.pdf = 1.f;
	result.type.clear_set(bxdf::Type::Specular_transmission);

	return 1.f;
}

math::float3 Sample::evaluate(const math::float3& /*wi*/, float& pdf) const {
	pdf = 0.f;
	return math::float3::identity;
}

math::float3 Sample::emission() const {
	return math::float3::identity;
}

math::float3 Sample::attenuation() const {
	return attenuation_;
}

void Sample::sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const {
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

bool Sample::is_pure_emissive() const {
	return false;
}

bool Sample::is_translucent() const {
	return false;
}

void Sample::set(const math::float3& color, float attenuation_distance, float ior, float ior_outside) {
	color_ = color;
	attenuation_ = material::Sample::attenuation(color, attenuation_distance);
	ior_ = ior;
	ior_outside_ = ior_outside;
}

}}}
