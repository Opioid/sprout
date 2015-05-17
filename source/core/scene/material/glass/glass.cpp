#include "glass.hpp"
#include "scene/material/ggx/ggx.inl"
#include "sampler/sampler.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace glass {

BRDF::BRDF(const Sample& sample) : BXDF(sample) {}

math::float3 BRDF::evaluate(const math::float3& /*wi*/) const {
	return math::float3::identity;
}

math::float3 fresnel(const math::float3& wi, const math::float3& wo) {
	math::float3 h = math::normalized(wo + wi);
	float wo_dot_h = math::dot(wo, h);

	math::float3 f0(0.03f, 0.03f, 0.03f);
	return ggx::f(wo_dot_h, f0);
}

math::float3 BRDF::importance_sample(sampler::Sampler& /*sampler*/, math::float3& wi, float& pdf) const {
	math::float3 n = sample_.n_;

	if (!sample_.same_hemisphere(sample_.wo_)) {
		n *= -1.f;
	}

	wi = math::normalized(math::reflect(n, -sample_.wo_));

	pdf = 1.f;

	return fresnel(wi, sample_.wo_);
}

BTDF::BTDF(const Sample& sample) : BXDF(sample) {}

math::float3 BTDF::evaluate(const math::float3& /*wi*/) const {
	return math::float3::identity;
}

math::float3 BTDF::importance_sample(sampler::Sampler& /*sampler*/, math::float3& wi, float& pdf) const {
	float etat = sample_.ior_;
	float eta  = 1.f / etat;

	math::float3 n = sample_.n_;

	math::float3 incident = -sample_.wo_;

	float cosi = -math::dot(incident, n);

	if (cosi < 0.f) {
		cosi = -cosi;
		n *= -1.f;
		eta = etat / 1.f;
	}

	float cost2 = 1.f - eta * eta * (1.f - cosi * cosi);
	if (cost2 < 0.f) {
		pdf = 0.f;
		return math::float3::identity;
	}

	math::float3 t = eta * incident + (eta * cosi - std::sqrt(cost2)) * n;
	wi = math::normalized(t);

	math::float3 f = fresnel(math::normalized(math::reflect(n, -sample_.wo_)), sample_.wo_);

	pdf = 1.f;

	return (math::float3(1.f, 1.f, 1.f) - f) * sample_.color_;
//	return fresnel * sample_.color_;
}

Sample::Sample() : brdf_(*this), btdf_(*this) {}

math::float3 Sample::evaluate(const math::float3& /*wi*/) const {
	return math::float3::identity;
}

math::float3 Sample::emission() const {
	return math::float3::identity;
}

void Sample::sample_evaluate(sampler::Sampler& sampler, Result& result) const {
	float p = sampler.generate_sample1d(0);

	if (p < 0.5f) {
		result.reflection = brdf_.importance_sample(sampler, result.wi, result.pdf);
		result.pdf *= 0.5f;
	} else {
		result.reflection = btdf_.importance_sample(sampler, result.wi, result.pdf);
		result.pdf *= 0.5f;
	}

//	result.reflection = brdf_.importance_sample(sampler, result.wi, result.pdf);

//	result.reflection = btdf_.importance_sample(sampler, result.wi, result.pdf);
}

void Sample::set(const math::float3& color, float ior) {
	color_ = color;
	ior_   = ior;
}

Glass::Glass(Sample_cache<Sample>& cache, std::shared_ptr<image::Image> mask) : Material(cache, mask) {}

math::float3 Glass::sample_emission() const {
	return math::float3::identity;
}

math::float3 Glass::average_emission() const {
	return math::float3::identity;
}

}}}
