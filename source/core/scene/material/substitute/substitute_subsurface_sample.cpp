#include "substitute_subsurface_sample.hpp"
#include "substitute_base_sample.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/bssrdf.hpp"
#include "scene/material/material_sample.inl"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector4.inl"
#include "base/math/sampling/sampling.hpp"

namespace scene::material::substitute {

bxdf::Result Sample_subsurface::evaluate(const float3& wi) const {
	if (!same_hemisphere(wo_)) {
		return { float3::identity(), 0.f };
	}

	const float3 h = math::normalize(wo_ + wi);
	const float wo_dot_h = clamp_dot(wo_, h);

	auto result = layer_.base_evaluate(wi, wo_, h, wo_dot_h);
	result.pdf *= 0.5f;
	return result;
}

void Sample_subsurface::sample(sampler::Sampler& sampler, bxdf::Sample& result) const {
	const bool same_side = same_hemisphere(wo_);

	const float p = sampler.generate_sample_1D();

	if (same_side) {
		if (p < 0.5f) {
			refract(same_side, layer_, sampler, result);
		} else {
			if (p < 0.75f) {
				layer_.diffuse_sample(wo_, sampler, result);
			} else {
				layer_.specular_sample(wo_, sampler, result);
			}
		}
	} else {
		Layer tmp_layer = layer_;
		tmp_layer.n_ *= -1.f;

		if (p < 0.5f) {
			refract(same_side, tmp_layer, sampler, result);
		} else {
			reflect_internally(same_side, tmp_layer, sampler, result);
		}
	}

	result.pdf *= 0.5f;
}

float3 Sample_subsurface::absorption_coefficient() const {
	return bssrdf_.absorption_coefficient();
}

BSSRDF Sample_subsurface::bssrdf() const {
	return bssrdf_;
}

void Sample_subsurface::set(const float3& absorption_coefficient,
							const float3& scattering_coefficient,
							float anisotropy,
							const IOR& ior) {
	bssrdf_.set(absorption_coefficient, scattering_coefficient, anisotropy);
	ior_ = ior;
}

bool Sample_subsurface::is_sss() const {
	return bssrdf_.is_scattering();
}

void Sample_subsurface::refract(bool same_side, const Layer& layer, sampler::Sampler& sampler,
								bxdf::Sample& result) const {
	IOR tmp_ior;

	if (same_side) {
		tmp_ior.ior_i_ = ior_.ior_i_;
		tmp_ior.ior_o_ = ior_.ior_o_;
		tmp_ior.eta_i_ = ior_.eta_i_;
	//	tmp_ior.eta_t_ = ior_.eta_t_;
		tmp_ior.sqrt_eta_t = ior_.sqrt_eta_t;
	} else {
		tmp_ior.ior_i_ = ior_.ior_o_;
		tmp_ior.ior_o_ = ior_.ior_i_;
		tmp_ior.eta_i_ = ior_.eta_t_;
	//	tmp_ior.eta_t_ = ior_.eta_i_;
		tmp_ior.sqrt_eta_t = ior_.sqrt_eta_i;
	}

	const float n_dot_wo = layer.clamp_abs_n_dot(wo_);

	const float sint2 = (tmp_ior.eta_i_ * tmp_ior.eta_i_) * (1.f - n_dot_wo * n_dot_wo);

	if (sint2 >= 1.f) {
		result.pdf = 0.f;
		return;
	}

	const float n_dot_t = std::sqrt(1.f - sint2);

	// fresnel has to be the same value that would have been computed by BRDF
	const fresnel::Schlick_refract schlick(layer.f0_, tmp_ior.sqrt_eta_t);
	const float n_dot_wi = ggx::Isotropic::refract(wo_, n_dot_wo, n_dot_t, layer, tmp_ior,
												   schlick, sampler, result);

	result.reflection *= n_dot_wi;
}

void Sample_subsurface::reflect_internally(bool same_side, const Layer& layer,
										   sampler::Sampler& sampler, bxdf::Sample& result) const {
	float eta_i;
	float sqrt_eta_t;

	if (same_side) {
		eta_i = ior_.eta_i_;
		sqrt_eta_t = ior_.sqrt_eta_t;
	} else {
		eta_i = ior_.eta_t_;
		sqrt_eta_t = ior_.sqrt_eta_i;
	}

	const float n_dot_wo = layer.clamp_abs_n_dot(wo_);

	const float sint2 = (eta_i * eta_i) * (1.f - n_dot_wo * n_dot_wo);

	const fresnel::Schlick_refract_conditional schlick(layer.f0_, sqrt_eta_t, sint2 > 1.f);

	const float n_dot_wi = ggx::Isotropic::reflect(wo_, n_dot_wo, layer,
												   schlick, sampler, result);

	SOFT_ASSERT(testing::check(result, wo_, layer));

	result.reflection *= n_dot_wi;
}

}
