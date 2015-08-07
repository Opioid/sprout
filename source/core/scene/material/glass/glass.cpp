#include "glass.hpp"
#include "scene/material/ggx/ggx.inl"
#include "sampler/sampler.hpp"
#include "base/math/vector.inl"

// #include <iostream>

namespace scene { namespace material { namespace glass {

// Adapted from https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/#more-1921
float fresnel_dielectric(float n_dot_wi, float n_dot_wo, float eta) {
	float t1 = eta * n_dot_wo;
	float t2 = eta * n_dot_wi;

	float rs = (n_dot_wi + t1) / (n_dot_wi - t1);
	float rp = (n_dot_wo + t2) / (n_dot_wo - t2);

	return 0.5f * (rs * rs + rp * rp);
}

float fresnel_dielectric_holgerusan(float eta_i, float eta_t, float cos_theta_i, float cos_theta_t) {
	float r_p = (eta_t * cos_theta_i + eta_i * cos_theta_t) / (eta_t * cos_theta_i - eta_i * cos_theta_t);
	float r_o = (eta_i * cos_theta_i + eta_t * cos_theta_t) / (eta_i * cos_theta_i - eta_t * cos_theta_t);

	return 0.5f * (r_p * r_p + r_o * r_o);
}

BRDF::BRDF(const Sample& sample) : BxDF(sample) {}

math::float3 BRDF::evaluate(const math::float3& /*wi*/, float /*n_dot_wi*/) const {
	return math::float3::identity;
}

float BRDF::pdf(const math::float3& /*wi*/, float /*n_dot_wi*/) const {
	return 0.f;
}

float BRDF::importance_sample(sampler::Sampler& /*sampler*/, BxDF_result& result) const {
	math::float3 n = sample_.n_;
	float eta = 1.f / sample_.ior_;

	if (!sample_.same_hemisphere(sample_.wo_)) {
		n *= -1.f;
		eta = sample_.ior_;
	}

	float n_dot_wo = math::saturate(math::dot(n, sample_.wo_));

	result.wi = math::normalized(2.f * n_dot_wo * n - sample_.wo_);

	float sint2 = (eta * eta) * (1.f - n_dot_wo * n_dot_wo);

	float f;
	if (sint2 > 1.f) {
		f = 1.f;
	} else {
		float n_dot_t = -std::sqrt(1.f - sint2);

		// fresnel has to be the same value that would have been computed by BRDF

	//	f = fresnel_dielectric(n_dot_t, n_dot_wo, eta);

		f = fresnel_dielectric_holgerusan(eta, 1.f / eta, n_dot_wo, n_dot_t);
	}

	result.reflection = math::float3(f);
	result.pdf = 1.f;
	result.type.clear_set(BxDF_type::Specular_reflection);

	return 1.f;
}

BTDF::BTDF(const Sample& sample) : BxDF(sample) {}

math::float3 BTDF::evaluate(const math::float3& /*wi*/, float /*n_dot_wi*/) const {
	return math::float3::identity;
}

float BTDF::pdf(const math::float3& /*wi*/, float /*n_dot_wi*/) const {
	return 0.f;
}

float BTDF::importance_sample(sampler::Sampler& /*sampler*/, BxDF_result& result) const {
	math::float3 n = sample_.n_;
	float eta = 1.f / sample_.ior_;

	if (!sample_.same_hemisphere(sample_.wo_)) {
		n *= -1.f;
		eta = sample_.ior_;
	}

	float n_dot_wo = math::saturate(math::dot(n, sample_.wo_));

	float sint2 = (eta * eta) * (1.f - n_dot_wo * n_dot_wo);

	if (sint2 > 1.f) {
		result.pdf = 0.f;
		return 0.f;
	}

	float n_dot_t = -std::sqrt(1.f - sint2);
	result.wi = (eta * n_dot_wo + n_dot_t) * n - eta * sample_.wo_;

	// fresnel has to be the same value that would have been computed by BRDF
//	float f = fresnel_dielectric(n_dot_t, n_dot_wo, eta);

	float f = fresnel_dielectric_holgerusan(eta, 1.f / eta, n_dot_wo, n_dot_t);

	result.reflection = (1.f - f) * sample_.color_;
	result.pdf = 1.f;
	result.type.clear_set(BxDF_type::Specular_transmission);

	return 1.f;
}

Sample::Sample() : brdf_(*this), btdf_(*this) {}

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

void Sample::sample_evaluate(sampler::Sampler& sampler, BxDF_result& result) const {
	float p = sampler.generate_sample_1D();

	if (p < 0.5f) {
		brdf_.importance_sample(sampler, result);
		result.pdf *= 0.5f;
	} else {
		btdf_.importance_sample(sampler, result);
		result.pdf *= 0.5f;
	}

//	brdf_.importance_sample(sampler, result);

//	btdf_.importance_sample(sampler, result);
}

bool Sample::is_pure_emissive() const {
	return false;
}

void Sample::set(const math::float3& color, const math::float3& attenuation, float ior) {
	color_ = color;
	attenuation_ = attenuation;
	ior_ = ior;
}

Glass::Glass(Sample_cache<Sample>& cache, std::shared_ptr<image::texture::Texture_2D> mask) :
	Material(cache, mask) {}

math::float3 Glass::sample_emission(math::float2 /*uv*/, const image::texture::sampler::Sampler_2D& /*sampler*/) const {
	return math::float3::identity;
}

math::float3 Glass::average_emission() const {
	return math::float3::identity;
}

const image::texture::Texture_2D* Glass::emission_map() const {
	return nullptr;
}

}}}
