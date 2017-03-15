#include "glass_rough_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"

#include "scene/material/material_test.hpp"
#include "base/debug/assert.hpp"

namespace scene { namespace material { namespace glass {

const material::Sample::Layer& Sample_rough::base_layer() const {
	return layer_;
}

float3 Sample_rough::evaluate(float3_p wi, float& pdf) const {
	if (!same_hemisphere(wo_)) {
		// only handling reflection for now
		pdf = 0.f;
		return float3::identity();
	}

	float n_dot_wi = layer_.clamped_n_dot(wi);
	float n_dot_wo = layer_.clamped_n_dot(wo_);

	float sint2 = (layer_.eta_i_ * layer_.eta_i_) * (1.f - n_dot_wo * n_dot_wo);

	float f;
	if (sint2 > 1.f) {
		f = 1.f;
	} else {
		float n_dot_t = std::sqrt(1.f - sint2);

		// fresnel has to be the same value that would have been computed by BRDF
		f = fresnel::dielectric(n_dot_wo, n_dot_t, layer_.eta_i_, layer_.eta_t_);
	}

	float3 h = math::normalized(wo_ + wi);
	float wo_dot_h = math::clamp(math::dot(wo_, h), 0.00001f, 1.f);

	const float n_dot_h = math::saturate(math::dot(layer_.n_, h));

	float3 f3(f);
	fresnel::Constant constant(f3);
	float3 reflection = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h,
												   layer_, constant, pdf);

	return n_dot_wi * reflection;

}

void Sample_rough::sample(sampler::Sampler& sampler, bxdf::Result& result) const {
	float p = sampler.generate_sample_1D();

	if (p < 0.5f) {
		float n_dot_wi = BSDF::reflect(*this, layer_, sampler, result);
		result.pdf *= 0.5f;
		result.reflection *= n_dot_wi;
	} else {
		float n_dot_wi = BSDF::refract(*this, layer_, sampler, result);
		result.pdf *= 0.5f;
		result.reflection *= n_dot_wi;
	}

//	float n_dot_wi = BSDF::reflect(*this, layer_, sampler, result);
//	result.reflection *= n_dot_wi;

//	float n_dot_wi = BSDF::refract(*this, layer_, sampler, result);
//	result.reflection *= n_dot_wi;
}

float3 Sample_rough::radiance() const {
	return float3::identity();
}

float3 Sample_rough::attenuation() const {
	return layer_.attenuation_;
}

float Sample_rough::ior() const {
	return layer_.ior_i_;
}

bool Sample_rough::is_pure_emissive() const {
	return false;
}

bool Sample_rough::is_transmissive() const {
	return true;
}

bool Sample_rough::is_translucent() const {
	return false;
}

void Sample_rough::Layer::set(float3_p refraction_color, float3_p absorption_color,
							  float attenuation_distance, float ior,
							  float ior_outside, float a_a2) {
	color_ = refraction_color;
	attenuation_ = material::Sample::attenuation(absorption_color, attenuation_distance);
	ior_i_ = ior;
	ior_o_ = ior_outside;
	eta_i_ = ior_outside / ior;
	eta_t_ = ior / ior_outside;
	a2_ = a_a2;
}

float Sample_rough::BSDF::reflect(const Sample& sample, const Layer& layer,
								  sampler::Sampler& sampler, bxdf::Result& result) {
	Layer tmp = layer;

	if (!sample.same_hemisphere(sample.wo_)) {
		tmp.n_ *= -1.f;
		tmp.ior_i_ = layer.ior_o_;
		tmp.ior_o_ = layer.ior_i_;
		tmp.eta_i_ = layer.eta_t_;
		tmp.eta_t_ = layer.eta_i_;
	}

	float n_dot_wo = tmp.clamped_n_dot(sample.wo_);

	float sint2 = (tmp.eta_i_ * tmp.eta_i_) * (1.f - n_dot_wo * n_dot_wo);

	float f;
	if (sint2 > 1.f) {
		f = 1.f;
	} else {
		float n_dot_t = std::sqrt(1.f - sint2);

		// fresnel has to be the same value that would have been computed by BRDF
		f = fresnel::dielectric(n_dot_wo, n_dot_t, tmp.eta_i_, tmp.eta_t_);
	}

	float3 f3(f);
	fresnel::Constant constant(f3);
	float n_dot_wi = ggx::Isotropic::reflect(sample.wo_, n_dot_wo, tmp,
											 constant, sampler, result);

	SOFT_ASSERT(testing::check(result, sample.wo_, layer));

	return n_dot_wi;
}

float Sample_rough::BSDF::refract(const Sample& sample, const Layer& layer,
								  sampler::Sampler& sampler, bxdf::Result& result) {
	Layer tmp = layer;

	if (!sample.same_hemisphere(sample.wo_)) {
		tmp.n_ *= -1.f;
		tmp.ior_i_ = layer.ior_o_;
		tmp.ior_o_ = layer.ior_i_;
		tmp.eta_i_ = layer.eta_t_;
		tmp.eta_t_ = layer.eta_i_;
	}

	float n_dot_wo = tmp.clamped_n_dot(sample.wo_);

	float sint2 = (tmp.eta_i_ * tmp.eta_i_) * (1.f - n_dot_wo * n_dot_wo);

	if (sint2 > 1.f) {
		result.pdf = 0.f;
		return 0.f;
	}

	float n_dot_t = std::sqrt(1.f - sint2);

	// fresnel has to be the same value that would have been computed by BRDF
	float f = fresnel::dielectric(n_dot_wo, n_dot_t, tmp.eta_i_, tmp.eta_t_);

	float3 f3(1.f - f);
	fresnel::Constant constant(f3);
	float n_dot_wi = ggx::Isotropic::refract(sample.wo_, n_dot_wo, n_dot_t, tmp,
											 constant, sampler, result);

	SOFT_ASSERT(testing::check(result, sample.wo_, layer));

	return n_dot_wi;
}

}}}
