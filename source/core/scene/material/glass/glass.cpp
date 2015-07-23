#include "glass.hpp"
#include "scene/material/ggx/ggx.inl"
#include "sampler/sampler.hpp"
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

	float wo_dot_h = math::dot(sample_.wo_, n);

	result.wi = math::normalized(2.f * wo_dot_h * n - sample_.wo_);

	float n_dot_wo = math::dot(n, sample_.wo_);

	float sint2 = (eta * eta) * (1.f - n_dot_wo * n_dot_wo);

	float f;
	if (sint2 > 1.f) {
		f = 1.f;
	} else {
		float n_dot_wi = -std::sqrt(1.f - sint2);

		// fresnel has to be the same value that would have been computed by BRDF
		f = fresnel_dielectric(n_dot_wi, n_dot_wo, eta);
	}

	result.reflection = math::float3(f);//math::float3(ggx::f(wo_dot_h, sample_.f0_));
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

	float n_dot_wo = math::dot(n, sample_.wo_);

	float sint2 = (eta * eta) * (1.f - n_dot_wo * n_dot_wo);

	if (sint2 > 1.f) {
		result.pdf = 0.f;
		return 0.f;
	}

	float n_dot_wi = -std::sqrt(1.f - sint2);
	math::float3 t = (eta * n_dot_wo + n_dot_wi) * n - eta * sample_.wo_;
	result.wi = t;

	// fresnel has to be the same value that would have been computed by BRDF
	float f = fresnel_dielectric(n_dot_wi, n_dot_wo, eta);

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

void Sample::set(const math::float3& color, const math::float3& attenuation, float ior, float f0) {
	color_ = color;
	attenuation_ = attenuation;
	ior_ = ior;
	f0_  = f0;
}

Glass::Glass(Sample_cache<Sample>& cache, std::shared_ptr<image::texture::Texture_2D> mask) : Material(cache, mask) {}

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
