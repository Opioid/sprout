#include "substitute_subsurface_sample.hpp"
#include "substitute_base_sample.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/bssrdf.hpp"
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

	const float3 h = math::normalize(wo_ + wi);
	const float wo_dot_h = clamp_dot(wo_, h);

	return layer_.base_evaluate(wi, wo_, h, wo_dot_h, pdf);
}

void Sample_subsurface::sample(sampler::Sampler& sampler, bxdf::Result& result) const {
/*
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
			tmp_layer.pure_specular_sample(wo_, sampler, result);
		}
	}

	result.pdf *= 0.5f;
	*/

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

float3 Sample_subsurface::absorption_coeffecient() const {
	return absorption_coefficient_;
}

const BSSRDF& Sample_subsurface::bssrdf(Worker& worker) const {
	auto& bssrdf = worker.sample_cache().bssrdf();

	bssrdf.set(absorption_coefficient_, scattering_coefficient_);

	return bssrdf;
}

void Sample_subsurface::set(const float3& absorption_coefficient,
							const float3& scattering_coefficient,
							float ior, float ior_outside) {
	absorption_coefficient_ = absorption_coefficient;
	scattering_coefficient_ = scattering_coefficient;
	ior_.ior_i_ = ior;
	ior_.ior_o_ = ior_outside;
	ior_.eta_i_ = ior_outside / ior;
	ior_.eta_t_ = ior / ior_outside;
}

bool Sample_subsurface::is_sss() const {
	return true;
}

void Sample_subsurface::refract(bool same_side, const Layer& layer, sampler::Sampler& sampler,
								bxdf::Result& result) const {
	IOR tmp_ior;

	if (!same_side) {
		tmp_ior.ior_i_ = ior_.ior_o_;
		tmp_ior.ior_o_ = ior_.ior_i_;
		tmp_ior.eta_i_ = ior_.eta_t_;
	//	tmp_ior.eta_t_ = ior_.eta_i_;
	} else {
		tmp_ior.ior_i_ = ior_.ior_i_;
		tmp_ior.ior_o_ = ior_.ior_o_;
		tmp_ior.eta_i_ = ior_.eta_i_;
	//	tmp_ior.eta_t_ = ior_.eta_t_;
	}

	const float n_dot_wo = layer.clamp_abs_n_dot(wo_); //layer.clamp_n_dot(wo_);

	const float sint2 = (tmp_ior.eta_i_ * tmp_ior.eta_i_) * (1.f - n_dot_wo * n_dot_wo);

	if (sint2 > 1.f) {
		result.pdf = 0.f;
		return;
	}

	const float n_dot_t = std::sqrt(1.f - sint2);

	// fresnel has to be the same value that would have been computed by BRDF
//	const float f = fresnel::dielectric(n_dot_wo, n_dot_t, tmp_ior.eta_i_, tmp_ior.eta_t_);
//	const fresnel::Constant constant(f);

	const fresnel::Schlick schlick(layer.f0_);
	const float n_dot_wi = ggx::Isotropic::refract(wo_, n_dot_wo, n_dot_t, layer, tmp_ior,
												   schlick, sampler, result);

	result.reflection *= n_dot_wi;
}

void Sample_subsurface::reflect_internally(bool same_side, const Layer& layer,
										  sampler::Sampler& sampler, bxdf::Result& result) const {
	IOR tmp_ior;

	if (!same_side) {
		tmp_ior.ior_i_ = ior_.ior_o_;
		tmp_ior.ior_o_ = ior_.ior_i_;
		tmp_ior.eta_i_ = ior_.eta_t_;
	//	tmp_ior.eta_t_ = ior_.eta_i_;
	} else {
		tmp_ior.ior_i_ = ior_.ior_i_;
		tmp_ior.ior_o_ = ior_.ior_o_;
		tmp_ior.eta_i_ = ior_.eta_i_;
	//	tmp_ior.eta_t_ = ior_.eta_t_;
	}

	const float n_dot_wo = layer.clamp_abs_n_dot(wo_); //tmp.clamp_n_dot(sample.wo());

	const float sint2 = (tmp_ior.eta_i_ * tmp_ior.eta_i_) * (1.f - n_dot_wo * n_dot_wo);

	const fresnel::Schlick_conditional schlick(layer.f0_, sint2 > 1.f);

	const float n_dot_wi = ggx::Isotropic::reflect(wo_, n_dot_wo, layer,
												   schlick, sampler, result);

	SOFT_ASSERT(testing::check(result, wo_, layer));

	result.reflection *= n_dot_wi;
}

}}}
