#include "glass_rough_sample.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_attenuation.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"

#include "scene/material/material_test.hpp"
#include "base/debug/assert.hpp"

#include <iostream>
#include "base/math/print.hpp"

namespace scene::material::glass {

const material::Sample::Layer& Sample_rough::base_layer() const {
	return layer_;
}

bxdf::Result Sample_rough::evaluate(const float3& wi) const {
	if (!same_hemisphere(wo_)) {
		Layer tmp = layer_;
		tmp.n_     = -layer_.n_;
		tmp.ior_i_ = layer_.ior_o_;
		tmp.ior_o_ = layer_.ior_i_;
		tmp.eta_i_ = layer_.eta_t_;
		tmp.eta_t_ = layer_.eta_i_;

		const float n_dot_wi = tmp.clamp_reverse_n_dot(wi);

		const float n_dot_wo = tmp.clamp_abs_n_dot(wo_);

		const float sint2 = (tmp.eta_i_ * tmp.eta_i_) * (1.f - n_dot_wo * n_dot_wo);

		if (sint2 >= 1.f) {
			return { float3::identity(), 0.f };
		}

		const float n_dot_t = std::sqrt(1.f - sint2);

			// fresnel has to be the same value that would have been computed by BRDF
		float f = fresnel::dielectric(n_dot_wo, n_dot_t, tmp.eta_i_, tmp.eta_t_);

	//	const float3 wi = math::normalize((ior.eta_i_ * wo_dot_h - n_dot_t) * h - ior.eta_i_ * wo);


		const float3 swi = math::normalize((tmp.eta_i_ * n_dot_wo - n_dot_t) * tmp.n_ - tmp.eta_i_ * wo_);

		const float wi_dot_swi = math::dot(wi, swi);

		const float3 swo = math::normalize((tmp.eta_i_ * -n_dot_wi - n_dot_t) * -tmp.n_ - tmp.eta_i_ * wi);

		const float wo_dot_swo = math::dot(wo_, swo);

		const float pre_calc_wo_dot_h = 0.817036331f;


		const float3 h = math::normalize((wi + tmp.eta_i_ * wo_) / (tmp.eta_i_ * n_dot_wo - n_dot_t));
	//	const float3 h = math::normalize((wi + tmp.eta_i_ * wo_) / (tmp.eta_i_ * pre_calc_wo_dot_h - n_dot_t));
	//	const float3 h = float3(0.00700425f, -0.00560187f, -0.99996f);


		const float wo_dot_h = clamp_dot(wo_, h);

		const float wi_dot_h = clamp_reverse_dot(wi, h);

		const float n_dot_h = math::saturate(math::dot(tmp.n_, h));

		std::cout << "evaluate:" << std::endl;
		std::cout << "h: " << h << std::endl;

		const fresnel::Constant constant(f);
		const auto ggx = ggx::Isotropic::refraction(n_dot_wi, n_dot_wo, wi_dot_h, wo_dot_h, n_dot_h,
													tmp, constant);

		return { n_dot_wi * ggx.reflection, 0.5f * ggx.pdf };

	}

	const float n_dot_wi = layer_.clamp_n_dot(wi);
	const float n_dot_wo = layer_.clamp_abs_n_dot(wo_);

	const float sint2 = (layer_.eta_i_ * layer_.eta_i_) * (1.f - n_dot_wo * n_dot_wo);

	float f;
	if (sint2 >= 1.f) {
		f = 1.f;
	} else {
		const float n_dot_t = std::sqrt(1.f - sint2);

		// fresnel has to be the same value that would have been computed by BRDF
		f = fresnel::dielectric(n_dot_wo, n_dot_t, layer_.eta_i_, layer_.eta_t_);
	}

	const float3 h = math::normalize(wo_ + wi);
	const float wo_dot_h = clamp_dot(wo_, h);

	const float n_dot_h = math::saturate(math::dot(layer_.n_, h));

	const fresnel::Constant constant(f);
	const auto ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h,
												layer_, constant);

	return { n_dot_wi * ggx.reflection, 0.5f * ggx.pdf };
}

void Sample_rough::sample(sampler::Sampler& sampler, bxdf::Sample& result) const {
	const float p = sampler.generate_sample_1D();

	if (p < 0.5f) {
		const float n_dot_wi = BSDF::reflect(*this, layer_, sampler, result);
		result.pdf *= 0.5f;
		result.reflection *= n_dot_wi;
	} else {
		const float n_dot_wi = BSDF::refract(*this, layer_, sampler, result);
		result.pdf *= 0.5f;
		result.reflection *= n_dot_wi;
	}

	result.wavelength = 0.f;
}

float3 Sample_rough::absorption_coefficient() const {
	return layer_.absorption_coefficient_;
}

float Sample_rough::ior() const {
	return layer_.ior_i_;
}

bool Sample_rough::is_transmissive() const {
	return true;
}

void Sample_rough::Layer::set(const float3& refraction_color, const float3& absorption_color,
							  float attenuation_distance, float ior,
							  float ior_outside, float alpha) {
	color_ = refraction_color;
	absorption_coefficient_ = material::extinction_coefficient(absorption_color,
															   attenuation_distance);
	ior_i_ = ior;
	ior_o_ = ior_outside;
	eta_i_ = ior_outside / ior;
	eta_t_ = ior / ior_outside;
	alpha_ = alpha;
	alpha2_ = alpha * alpha;
}

float Sample_rough::BSDF::reflect(const Sample& sample, const Layer& layer,
								  sampler::Sampler& sampler, bxdf::Sample& result) {
	Layer tmp = layer;

	if (!sample.same_hemisphere(sample.wo())) {
		tmp.n_	   = -layer.n_;
		tmp.ior_i_ = layer.ior_o_;
		tmp.ior_o_ = layer.ior_i_;
		tmp.eta_i_ = layer.eta_t_;
		tmp.eta_t_ = layer.eta_i_;
	}

	const float n_dot_wo = tmp.clamp_abs_n_dot(sample.wo()); //tmp.clamp_n_dot(sample.wo());

	const float sint2 = (tmp.eta_i_ * tmp.eta_i_) * (1.f - n_dot_wo * n_dot_wo);

	float f;
	if (sint2 >= 1.f) {
		f = 1.f;
	} else {
		const float n_dot_t = std::sqrt(1.f - sint2);

		// fresnel has to be the same value that would have been computed by BRDF
		f = fresnel::dielectric(n_dot_wo, n_dot_t, tmp.eta_i_, tmp.eta_t_);
	}

	const fresnel::Constant constant(f);
	const float n_dot_wi = ggx::Isotropic::reflect(sample.wo(), n_dot_wo, tmp,
												   constant, sampler, result);

	SOFT_ASSERT(testing::check(result, sample.wo(), layer));

	return n_dot_wi;
}

float Sample_rough::BSDF::refract(const Sample& sample, const Layer& layer,
								  sampler::Sampler& sampler, bxdf::Sample& result) {
	Layer tmp = layer;

	if (!sample.same_hemisphere(sample.wo())) {
		tmp.n_	   = -layer.n_;
		tmp.ior_i_ = layer.ior_o_;
		tmp.ior_o_ = layer.ior_i_;
		tmp.eta_i_ = layer.eta_t_;
		tmp.eta_t_ = layer.eta_i_;
	}

	const float n_dot_wo = tmp.clamp_abs_n_dot(sample.wo()); //tmp.clamp_n_dot(sample.wo());

	const float sint2 = (tmp.eta_i_ * tmp.eta_i_) * (1.f - n_dot_wo * n_dot_wo);

	if (sint2 >= 1.f) {
		result.pdf = 0.f;
		return 0.f;
	}

	const float n_dot_t = std::sqrt(1.f - sint2);

	// fresnel has to be the same value that would have been computed by BRDF
	const float f = fresnel::dielectric(n_dot_wo, n_dot_t, tmp.eta_i_, tmp.eta_t_);

	const fresnel::Constant constant(f);
	const float n_dot_wi = ggx::Isotropic::refract(sample.wo(), n_dot_wo, n_dot_t, tmp,
												   constant, sampler, result);

	result.reflection *= layer.color_;

	SOFT_ASSERT(testing::check(result, sample.wo(), layer));

	return n_dot_wi;
}

}
