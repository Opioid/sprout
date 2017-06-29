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

		result.type.clear_set(bxdf::Type::SSS);
	} else {
		if (p < 0.75f) {
			layer_.diffuse_sample(wo_, sampler, result);
		} else {
			layer_.specular_sample(wo_, sampler, result);
		}


	}

	result.pdf *= 0.5f;
}

void Sample_subsurface::sample_sss(sampler::Sampler& sampler, bxdf::Result& result) const {
	Layer tmp_layer = layer_;
	IOR tmp_ior;

	if (!same_hemisphere(wo_)) {
		tmp_layer.n_ *= -1.f;
		tmp_ior.ior_i_ = ior_.ior_o_;
		tmp_ior.ior_o_ = ior_.ior_i_;
		tmp_ior.eta_i_ = ior_.eta_t_;
	} else {
		tmp_ior.ior_i_ = ior_.ior_i_;
		tmp_ior.ior_o_ = ior_.ior_o_;
		tmp_ior.eta_i_ = ior_.eta_i_;
	}

	const float n_dot_wo = tmp_layer.clamped_n_dot(wo_);

	const float sint2 = (tmp_ior.eta_i_ * tmp_ior.eta_i_) * (1.f - n_dot_wo * n_dot_wo);

	if (sint2 > 1.f) {
		result.pdf = 0.f;
		return;
	}

	const float n_dot_t = std::sqrt(1.f - sint2);

	const fresnel::Schlick schlick(layer_.f0_);
	const float n_dot_wi = ggx::Isotropic::refract(wo_, n_dot_wo, n_dot_t, tmp_layer, tmp_ior,
												   schlick, sampler, result);

	result.reflection *= n_dot_wi;
}

void Sample_subsurface::set(float ior, float ior_outside) {
	ior_.ior_i_ = ior;
	ior_.ior_o_ = ior_outside;
	ior_.eta_i_ = ior_outside / ior;
	ior_.eta_t_ = ior / ior_outside;
}

}}}

