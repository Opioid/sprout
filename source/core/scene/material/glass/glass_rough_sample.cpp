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

namespace scene::material::glass {

const material::Sample::Layer& Sample_rough::base_layer() const {
	return layer_;
}

bxdf::Result Sample_rough::evaluate(f_float3 wi) const {
	if (!same_hemisphere(wo_)) {
		return { float3(0.f), 0.f };
	}

	float const n_dot_wi = layer_.clamp_n_dot(wi);
	float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

	float3 const h = math::normalize(wo_ + wi);
	float const wo_dot_h = clamp_dot(wo_, h);
	float const n_dot_h = math::saturate(math::dot(layer_.n_, h));

	const fresnel::Schlick schlick(layer_.f0_);
	auto const ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h,
												layer_, schlick);

	return { n_dot_wi * ggx.reflection, 0.5f * ggx.pdf };
}

void Sample_rough::sample(sampler::Sampler& sampler, bxdf::Sample& result) const {
//	float const p = sampler.generate_sample_1D();

//	if (p < 0.5f) {
//		float const n_dot_wi = BSDF::reflect(*this, layer_, sampler, result);
//		result.pdf *= 0.5f;
//		result.reflection *= n_dot_wi;

//	} else {
//		float const n_dot_wi = BSDF::refract(*this, layer_, sampler, result);
//		result.pdf *= 0.5f;
//		result.reflection *= n_dot_wi;
//	}

//	result.wavelength = 0.f;

	const bool same_side = same_hemisphere(wo_);

	float const p = sampler.generate_sample_1D();

	if (same_side) {
		if (p < 0.5f) {
			refract(same_side, layer_, sampler, result);
		} else {
			reflect(layer_, sampler, result);
		}
	} else {
		Layer tmp_layer = layer_;
		tmp_layer.n_ = -layer_.n_;

		if (p < 0.5f) {
			refract(same_side, tmp_layer, sampler, result);
		} else {
			reflect_internally(tmp_layer, sampler, result);
		}
	}

	result.pdf *= 0.5f;
	result.wavelength = 0.f;
}

bool Sample_rough::is_transmissive() const {
	return true;
}

void Sample_rough::set(float3 const& refraction_color, float3 const& absorption_color,
							  float attenuation_distance, float ior,
							  float ior_outside, float alpha) {
	layer_.color_ = refraction_color;
	layer_.absorption_coefficient_ = material::extinction_coefficient(absorption_color,
															   attenuation_distance);

	layer_.f0_ = fresnel::schlick_f0(ior_outside, ior);
	layer_.alpha_ = alpha;
	layer_.alpha2_ = alpha * alpha;


	ior_.ior_i_ = ior;
	ior_.ior_o_ = ior_outside;
	ior_.eta_i_ = ior_outside / ior;
	ior_.eta_t_ = ior / ior_outside;
}

void Sample_rough::reflect(Layer const& layer, sampler::Sampler& sampler,
						   bxdf::Sample& result) const {
	float const n_dot_wo = layer.clamp_abs_n_dot(wo_);

	const fresnel::Schlick schlick(layer.f0_);
	float const n_dot_wi = ggx::Isotropic::reflect(wo_, n_dot_wo, layer,
												   schlick, sampler, result);

	SOFT_ASSERT(testing::check(result, wo_, layer));

	result.reflection *= n_dot_wi;
}


void Sample_rough::reflect_internally(Layer const& layer, sampler::Sampler& sampler,
									  bxdf::Sample& result) const {
	IOR tmp_ior;

	tmp_ior.ior_i_ = ior_.ior_o_;
	tmp_ior.ior_o_ = ior_.ior_i_;
	tmp_ior.eta_i_ = ior_.eta_t_;

	float const n_dot_wo = layer.clamp_abs_n_dot(wo_);

	const fresnel::Schlick schlick(layer.f0_);
	float const n_dot_wi = ggx::Isotropic::reflect_internally(wo_, n_dot_wo, layer, tmp_ior,
															  schlick, sampler, result);

	SOFT_ASSERT(testing::check(result, wo_, layer));

	result.reflection *= n_dot_wi;
}

void Sample_rough::refract(bool same_side, Layer const& layer, sampler::Sampler& sampler,
						   bxdf::Sample& result) const {
	IOR tmp_ior;

	if (same_side) {
		tmp_ior.ior_i_ = ior_.ior_i_;
		tmp_ior.ior_o_ = ior_.ior_o_;
		tmp_ior.eta_i_ = ior_.eta_i_;
	} else {
		tmp_ior.ior_i_ = ior_.ior_o_;
		tmp_ior.ior_o_ = ior_.ior_i_;
		tmp_ior.eta_i_ = ior_.eta_t_;
	}

	float const n_dot_wo = layer.clamp_abs_n_dot(wo_);

	const fresnel::Schlick schlick(layer.f0_);
	float const n_dot_wi = ggx::Isotropic::refract(wo_, n_dot_wo, layer, tmp_ior,
												   schlick, sampler, result);

	result.reflection *= n_dot_wi * layer.color_;
}


}
