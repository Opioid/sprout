#include "substitute_subsurface_sample.hpp"
#include "substitute_base_sample.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector4.inl"
#include "base/math/sampling/sampling.hpp"

namespace scene { namespace material { namespace substitute {

float3 Sample_subsurface::evaluate(const float3& wi, float& pdf) const {
	if (!same_hemisphere(wo_)) {
		pdf = 0.f;
		return float3::identity();
	}

	const float3 h = math::normalized(wo_ + wi);
	const float wo_dot_h = clamped_dot(wo_, h);

	return layer_.base_evaluate(wi, wo_, h, wo_dot_h, pdf);
}

void Sample_subsurface::sample(sampler::Sampler& sampler, bxdf::Result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	const float p = sampler.generate_sample_1D();

//	if (p < 0.5f) {
//		layer_.diffuse_sample(wo_, sampler, result);
//	} else {
//		layer_.specular_sample(wo_, sampler, result);
//	}

	if (p < 0.5f) {
		sample_sss(sampler, result);

		result.type.set(bxdf::Type::SSS);
	} else {
		if (p < 0.75f) {
			layer_.diffuse_sample(wo_, sampler, result);
		} else {
			layer_.specular_sample(wo_, sampler, result);
		}

		result.pdf *= 0.5f;
	}


}

void Sample_subsurface::sample_sss(sampler::Sampler& sampler, bxdf::Result& result) const {
//	if (!same_hemisphere(wo_)) {
//		result.pdf = 0.f;
//		return;
//	}

	const float n_dot_wo = layer_.clamped_n_dot(wo_);

	const float sint2 = (eta_i_ * eta_i_) * (1.f - n_dot_wo * n_dot_wo);

	if (sint2 > 1.f) {
		result.pdf = 0.f;
		return;
	}

	const float n_dot_t = std::sqrt(1.f - sint2);

	// Roughness zero will always have zero specular term (or worse NaN)
	SOFT_ASSERT(layer_.a2_ >= Min_a2);

	const float2 xi = sampler.generate_sample_2D();

	const float a2 = layer_.a2_;
	const float n_dot_h_squared = (1.f - xi[1]) / ((a2 - 1.f) * xi[1] + 1.f);
	const float sin_theta = std::sqrt(1.f - n_dot_h_squared);
	const float n_dot_h   = std::sqrt(n_dot_h_squared);
	const float phi = (2.f * math::Pi) * xi[0];
	float sin_phi;
	float cos_phi;
	math::sincos(phi, sin_phi, cos_phi);

	const float3 is = float3(sin_theta * cos_phi, sin_theta * sin_phi, n_dot_h);
	const float3 h = math::normalized(layer_.tangent_to_world(is));

	const float wo_dot_h = clamped_dot(wo_, h);

	const float3 wi = math::normalized((eta_i_ * wo_dot_h - n_dot_t) * h - eta_i_ * wo_);

	const float n_dot_wi = layer_.reversed_clamped_n_dot(wi);

	const float d = ggx::distribution_isotropic(n_dot_h, a2);
	const float g = ggx::G_smith(n_dot_wi, n_dot_wo, a2);
	// fresnel has to be the same value that would have been computed by BRDF
	const fresnel::Schlick fresnel(layer_.f0_);
	const float3 f = 1.f - fresnel(wo_dot_h);

	const float3 refraction = d * g * f;

	const float factor = (wo_dot_h * wo_dot_h) / (n_dot_wi * n_dot_wo);

	float denom = (ior_i_ + ior_o_) * wo_dot_h;
	denom = denom * denom;

	const float ior_o_2 = ior_o_ * ior_o_;
	result.reflection = n_dot_wi * factor * ((ior_o_2 * refraction) / denom);

	result.pdf = 0.5f * ((d * n_dot_h) / (4.f * wo_dot_h));
	result.wi = wi;
}

void Sample_subsurface::set(float ior, float ior_outside) {
	ior_i_ = ior;
	ior_o_ = ior_outside;
	eta_i_ = ior_outside / ior;
	eta_t_ = ior / ior_outside;
}

}}}

