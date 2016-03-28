#include "substitute_sample_clearcoat.hpp"
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

#include <iostream>

namespace scene { namespace material { namespace substitute {

math::float3 Clearcoat::evaluate(const Sample_clearcoat& /*sample*/,
								 const math::float3& /*wi*/, float /*n_dot_wi*/) const {
	return math::float3_identity;
}

float Clearcoat::pdf(const Sample_clearcoat& /*sample*/, const math::float3& /*wi*/, float /*n_dot_wi*/) const {
	return 0.f;
}

float Clearcoat::importance_sample(const Sample_clearcoat& sample, sampler::Sampler& /*sampler*/,
								   bxdf::Result& result) const {
	math::float3 n = sample.n_;
	float eta_i = 1.f / sample.ior_;
	float eta_t = sample.ior_;

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

float Clearcoat::fresnel(const Sample_clearcoat& sample) const {
	math::float3 n = sample.n_;
	float eta_i = 1.f / sample.ior_;
	float eta_t = sample.ior_;

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

	float clearcoat = 1.f - clearcoat_.fresnel(*this);

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
	float diffuse_pdf = n_dot_wi * math::Pi_inv;
	// ----

	// Roughness zero will always have zero specular term (or worse NaN)
	if (0.f == a2_) {
		pdf = diffuse_pdf;
		return clearcoat * n_dot_wi * diffuse;
	}

	math::float3 h = math::normalized(wo_ + wi);

	float n_dot_h  = math::dot(n_, h);
	float wo_dot_h = math::dot(wo_, h);

	float clamped_a2 = ggx::clamp_a2(a2_);
	float d = ggx::distribution_isotropic(n_dot_h, clamped_a2);
	float g = ggx::geometric_shadowing(n_dot_wi, n_dot_wo, clamped_a2);
	math::float3 f = fresnel::schlick(wo_dot_h, f0_);

	math::float3 specular = d * g * f;

	// this helped in the past, but problem maybe caused by faulty sphere normals
//	float ggx_pdf     = d * n_dot_h / (4.f * std::max(wo_dot_h, 0.00001f));
	float ggx_pdf     = d * n_dot_h / (4.f * wo_dot_h);

	pdf = 0.5f * (diffuse_pdf + ggx_pdf);

//	if (math::contains_negative(diffuse) || math::contains_negative(specular)) {
//		std::cout << "substitute::Sample::evaluate()" << std::endl;
//	}

	return clearcoat * n_dot_wi * (diffuse + specular);
}

math::float3 Sample_clearcoat::emission() const {
	return emission_;
}

math::float3 Sample_clearcoat::attenuation() const {
	return math::float3(100.f, 100.f, 100.f);
}

float Sample_clearcoat::ior() const {
	return 0.f;
}

void Sample_clearcoat::sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	float p = sampler.generate_sample_1D();

	if (p < 0.5f) {
		clearcoat_.importance_sample(*this, sampler, result);
		result.pdf *= 0.5f;
	} else {
		float clearcoat = 1.f - clearcoat_.fresnel(*this);

		if (1.f == metallic_) {
			float n_dot_wo = clamped_n_dot_wo();
			float n_dot_wi = ggx_.importance_sample(*this, sampler, n_dot_wo, result);
			result.reflection *= clearcoat * n_dot_wi;
			result.pdf *= 0.5f;
		} else {
			float n_dot_wo = clamped_n_dot_wo();

			if (p < 0.75f) {
				float n_dot_wi = oren_nayar_.importance_sample(*this, sampler, n_dot_wo, result);

				float ggx_pdf;
				math::float3 ggx_reflection = ggx_.evaluate(*this, result.wi, n_dot_wi, n_dot_wo, ggx_pdf);

				result.reflection = clearcoat * n_dot_wi * (result.reflection + ggx_reflection);
				result.pdf = 0.25f * (result.pdf + ggx_pdf);

			} else {
				float n_dot_wi = ggx_.importance_sample(*this, sampler, n_dot_wo, result);

				float oren_nayar_pdf;
				math::float3 oren_nayar_reflection = oren_nayar_.evaluate(*this, result.wi, n_dot_wi, n_dot_wo,
																		  oren_nayar_pdf);

				result.reflection = clearcoat * n_dot_wi * (result.reflection + oren_nayar_reflection);
				result.pdf = 0.25f * (result.pdf + oren_nayar_pdf);
			}
		}
	}
}

bool Sample_clearcoat::is_pure_emissive() const {
	return false;
}

bool Sample_clearcoat::is_transmissive() const {
	return false;
}

bool Sample_clearcoat::is_translucent() const {
	return false;
}

void Sample_clearcoat::set(const math::float3& color, const math::float3& emission,
						   float constant_f0, float roughness, float metallic) {
	diffuse_color_ = (1.f - metallic) * color;
	f0_ = math::lerp(math::float3(constant_f0), color, metallic);
	emission_ = emission;

	float a = roughness * roughness;
	a2_ = a * a;

	metallic_ = metallic;

	ior_ = 1.5f;
}

}}}
