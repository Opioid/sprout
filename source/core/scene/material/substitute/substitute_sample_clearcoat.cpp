#include "substitute_sample_clearcoat.hpp"
#include "rendering/integrator/surface/integrator_helper.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "sampler/sampler.hpp"
#include "base/math/vector.inl"
#include "base/math/math.hpp"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace material { namespace substitute {

math::float3 Clearcoat::evaluate(const Sample_clearcoat& /*sample*/,
								 const math::float3& /*wi*/, float /*n_dot_wi*/) {
	return math::float3_identity;
}

float Clearcoat::pdf(const Sample_clearcoat& /*sample*/, const math::float3& /*wi*/, float /*n_dot_wi*/) {
	return 0.f;
}

float Clearcoat::importance_sample(const Sample_clearcoat& sample, sampler::Sampler& /*sampler*/,
								   bxdf::Result& result) {
	math::float3 n = sample.n_;
	float eta_i = 1.f / sample.clearcoat_ior_;
	float eta_t = sample.clearcoat_ior_;

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

		f = fresnel::dielectric_holgerusan(n_dot_wo, n_dot_t, eta_i, eta_t);
	}

	result.reflection = math::float3(f);
	result.pdf = 1.f;
	result.type.clear_set(bxdf::Type::Specular_reflection);

	return 1.f;
}

float Clearcoat::fresnel(const Sample_clearcoat& sample) {
	math::float3 n = sample.n_;
	float eta_i = 1.f / sample.clearcoat_ior_;
	float eta_t = sample.clearcoat_ior_;

	float n_dot_wo = math::saturate(math::dot(n, sample.wo_));

	float sint2 = (eta_i * eta_i) * (1.f - n_dot_wo * n_dot_wo);

	if (sint2 > 1.f) {
		return 1.f;
	} else {
		float n_dot_t = -std::sqrt(1.f - sint2);
		return fresnel::dielectric_holgerusan(n_dot_wo, n_dot_t, eta_i, eta_t);
	}
}

math::float3 Sample_clearcoat::evaluate(math::pfloat3 wi, float& pdf) const {
	if (!same_hemisphere(wo_)) {
		pdf = 0.f;
		return math::float3_identity;
	}

	float clearcoat = 1.f - Clearcoat::fresnel(*this);

	return clearcoat * base_evaluate(wi, pdf);
}

void Sample_clearcoat::sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	float p = sampler.generate_sample_1D();

	if (p < 0.5f) {
		Clearcoat::importance_sample(*this, sampler, result);
		result.pdf *= 0.5f;
	} else {
		if (1.f == metallic_) {
			pure_specular_importance_sample(sampler, result);
		} else {
			if (p < 0.75f) {
				diffuse_importance_sample(sampler, result);
			} else {
				specular_importance_sample(sampler, result);
			}
		}

		float clearcoat = 1.f - Clearcoat::fresnel(*this);
		result.reflection *= clearcoat;
		result.pdf *= 0.5f;
	}
}

void Sample_clearcoat::set(const math::float3& color, const math::float3& emission,
						   float constant_f0, float roughness, float metallic) {
	diffuse_color_ = (1.f - metallic) * color;
	f0_ = math::lerp(math::float3(constant_f0), color, metallic);
	emission_ = emission;

	float a = roughness * roughness;
	a2_ = a * a;

	metallic_ = metallic;

	clearcoat_ior_ = 1.5f;
}

}}}
