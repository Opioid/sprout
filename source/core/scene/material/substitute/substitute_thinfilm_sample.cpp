#include "substitute_thinfilm_sample.hpp"
#include "rendering/integrator/surface/integrator_helper.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/oren_nayar/oren_nayar.inl"
#include "sampler/sampler.hpp"
#include "base/math/vector.inl"
#include "base/math/math.hpp"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace material { namespace substitute {

math::float3 Sample_thinfilm::evaluate(math::pfloat3 wi, float& pdf) const {
	if (!same_hemisphere(wo_)) {
		pdf = 0.f;
		return math::float3_identity;
	}

	float n_dot_wi = std::max(math::dot(n_, wi),  0.00001f);
	float n_dot_wo = std::max(math::dot(n_, wo_), 0.00001f);

	float diffuse_pdf;
	math::float3 diffuse = oren_nayar::Isotropic::evaluate(wi, n_dot_wi, n_dot_wo,
														   *this, diffuse_pdf);

	// Roughness zero will always have zero specular term (or worse NaN)
	if (0.f == a2_) {
		pdf = 0.5f * diffuse_pdf;
		return n_dot_wi * diffuse;
	}

	math::float3 h = math::normalized(wo_ + wi);

	float n_dot_h  = math::dot(n_, h);
	float wo_dot_h = math::dot(wo_, h);

	float clamped_a2 = ggx::clamp_a2(a2_);
	float d = ggx::distribution_isotropic(n_dot_h, clamped_a2);
	float g = ggx::geometric_shadowing(n_dot_wi, n_dot_wo, clamped_a2);
	math::float3 f = fresnel::schlick(wo_dot_h, f0_);
	math::float3 t_f = fresnel::thinfilm(wo_dot_h, 1.f, thinfilm_ior_, ior_, thinfilm_thickness_);

	math::float3 specular = d * g * f;

	float ggx_pdf = d * n_dot_h / (4.f * wo_dot_h);

	pdf = 0.5f * (diffuse_pdf + ggx_pdf);

	return /*n_dot_wi **/  f;

	return n_dot_wi * (diffuse + specular);

//	math::float3 cl_f = fresnel::thinfilm(wo_dot_h, 1.f, thinfilm_ior_, ior_, thinfilm_thickness_);

//	return n_dot_wi * cl_f;

//	return n_dot_wi * (cl_f + (1.f - cl_f) * (diffuse + specular));
}

void Sample_thinfilm::sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	if (1.f == metallic_) {
		pure_specular_importance_sample(sampler, result);
	} else {
		float p = sampler.generate_sample_1D();

		if (p < 0.5f) {
			diffuse_importance_sample(sampler, result);
		} else {
			specular_importance_sample(sampler, result);
		}
	}
}

void Sample_thinfilm::set(math::pfloat3 color, math::pfloat3 radiance,
						  float constant_f0, float ior, float a2, float metallic,
						  float thinfilm_ior, float thinfilm_thickness) {
	diffuse_color_ = (1.f - metallic) * color;
	f0_ = math::lerp(math::float3(constant_f0), color, metallic);
	emission_ = radiance;

	ior_ = ior;

	a2_ = a2;

	metallic_ = metallic;

	thinfilm_ior_ = thinfilm_ior;
	thinfilm_thickness_ = thinfilm_thickness;
}

}}}
