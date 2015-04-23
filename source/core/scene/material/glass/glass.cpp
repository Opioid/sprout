#include "glass.hpp"
#include "scene/material/ggx/ggx.inl"
#include "sampler/sampler.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace glass {

BRDF::BRDF(const Sample& sample) : BXDF(sample) {}

math::float3 BRDF::evaluate(const math::float3& /*wi*/) const {
	return math::float3::identity;
}

math::float3 BRDF::importance_sample(sampler::Sampler& /*sampler*/, math::float3& wi, float& pdf) const {
	math::float3 n = sample_.n_;

	if (!sample_.same_hemisphere(sample_.wo_)) {
		n *= -1.f;
	}

	wi = math::normalized(math::reflect(n, -sample_.wo_));

	math::float3 h = math::normalized(sample_.wo_ + wi);
	float wo_dot_h = math::dot(sample_.wo_, h);

	math::float3 f0(0.03f, 0.03f, 0.03f);
	math::float3 fresnel = ggx::f(wo_dot_h, f0);

	pdf = 1.f;

	return fresnel;
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

	math::float3 h = math::normalized(sample_.wo_ + wi);
	float wo_dot_h = math::dot(sample_.wo_, h);

	math::float3 f0(0.03f, 0.03f, 0.03f);
	math::float3 fresnel = ggx::f(wo_dot_h, f0);

	// By convention our color already is the transmittance
//	math::float3 transmittance = math::float3(1.f, 1.f, 1.f) - sample_.color_;

	pdf = 1.f;

	return sample_.color_ * fresnel;
}

Sample::Sample() : brdf_(*this), btdf_(*this) {}

math::float3 Sample::evaluate(const math::float3& /*wi*/) const {
	return math::float3::identity;
}

math::float3 Sample::sample_evaluate(sampler::Sampler& sampler, math::float3& wi, float& pdf) const {
	float p = sampler.generate_sample1d(0);

	if (p < 0.5f) {
		math::float3 r = brdf_.importance_sample(sampler, wi, pdf);
		pdf *= 0.5f;
		return r;
	} else {
		math::float3 r = btdf_.importance_sample(sampler, wi, pdf);
		pdf *= 0.5f;
		return r;
	}

//	math::float3 r = brdf_.importance_sample(sampler, wi, pdf);
//	return r;

//	math::float3 r = btdf_.importance_sample(sampler, wi, pdf);
//	return r;
}

void Sample::set(const math::float3& color, float ior) {
	color_ = color;
	ior_   = ior;
}

Glass::Glass(Sample_cache<Sample>& cache) : Material(cache) {}

}}}
