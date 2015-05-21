#include "glass.hpp"
#include "scene/material/ggx/ggx.inl"
#include "sampler/sampler.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace glass {

BRDF::BRDF(const Sample& sample) : BxDF(sample) {}

math::float3 BRDF::evaluate(const math::float3& /*wi*/) const {
	return math::float3::identity;
}

math::float3 fresnel(const math::float3& wi, const math::float3& wo, float f0) {
	math::float3 h = math::normalized(wo + wi);
	float wo_dot_h = math::dot(wo, h);

	return ggx::f(wo_dot_h, math::float3(f0, f0, f0));
}

void BRDF::importance_sample(sampler::Sampler& /*sampler*/, BxDF_result& result) const {
	math::float3 n = sample_.n_;

	if (!sample_.same_hemisphere(sample_.wo_)) {
		n *= -1.f;
	}

	result.wi = math::normalized(math::reflect(n, -sample_.wo_));

	result.pdf = 1.f;

	result.reflection = fresnel(result.wi, sample_.wo_, sample_.f0_);

	result.type.clear_set(BxDF_type::Specular_reflection);
}

BTDF::BTDF(const Sample& sample) : BxDF(sample) {}

math::float3 BTDF::evaluate(const math::float3& /*wi*/) const {
	return math::float3::identity;
}

void BTDF::importance_sample(sampler::Sampler& /*sampler*/, BxDF_result& result) const {
	float eta  = 1.f / sample_.ior_;

	math::float3 n = sample_.n_;

	math::float3 incident = -sample_.wo_;

	float cosi = -math::dot(incident, n);

	if (cosi < 0.f) {
		cosi = -cosi;
		n *= -1.f;
		eta = sample_.ior_;
	}

	float cost2 = 1.f - eta * eta * (1.f - cosi * cosi);
	if (cost2 < 0.f) {
		result.pdf = 0.f;
		return;
	}

	math::float3 t = eta * incident + (eta * cosi - std::sqrt(cost2)) * n;
	result.wi = math::normalized(t);

	// fresnel has to be the same value that would have been computed by BRDF
	math::float3 f = fresnel(math::normalized(math::reflect(n, incident)), sample_.wo_, sample_.f0_);

	result.pdf = 1.f;

	result.reflection = (math::float3(1.f, 1.f, 1.f) - f) * sample_.color_;

	result.type.clear_set(BxDF_type::Specular_transmission);
}

Sample::Sample() : brdf_(*this), btdf_(*this) {}

math::float3 Sample::evaluate(const math::float3& /*wi*/) const {
	return math::float3::identity;
}

math::float3 Sample::emission() const {
	return math::float3::identity;
}

math::float3 Sample::attenuation() const {
	return attenuation_;
}

void Sample::sample_evaluate(sampler::Sampler& sampler, BxDF_result& result) const {
	float p = sampler.generate_sample1d(0);

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

void Sample::set(const math::float3& color, const math::float3& attenuation, float ior, float f0) {
	color_ = color;
	attenuation_ = attenuation;
	ior_ = ior;
	f0_  = f0;
}

Glass::Glass(Sample_cache<Sample>& cache, std::shared_ptr<image::Image> mask) : Material(cache, mask) {}

math::float3 Glass::sample_emission() const {
	return math::float3::identity;
}

math::float3 Glass::average_emission() const {
	return math::float3::identity;
}

}}}
