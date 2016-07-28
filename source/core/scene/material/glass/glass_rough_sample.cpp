#include "glass_rough_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"

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
		return math::float3_identity;
	}

	float n_dot_wi = layer_.clamped_n_dot(wi);
	float n_dot_wo = layer_.clamped_n_dot(wo_);

	float sint2 = (layer_.eta_i * layer_.eta_i) * (1.f - n_dot_wo * n_dot_wo);

	float f;
	if (sint2 > 1.f) {
		f = 1.f;
	} else {
		float n_dot_t = std::sqrt(1.f - sint2);

		// fresnel has to be the same value that would have been computed by BRDF
		f = fresnel::dielectric(n_dot_wo, n_dot_t, layer_.eta_i, layer_.eta_t);
	}

	float3 f3(f);
	fresnel::Constant constant(f3);
	float3 reflection = ggx::Isotropic::reflection(wi, wo_, n_dot_wi, n_dot_wo,
												   layer_, constant, pdf);

	return n_dot_wi * reflection;
}

void Sample_rough::sample(sampler::Sampler& sampler, bxdf::Result& result) const {
//	float p = sampler.generate_sample_1D();

//	if (p < 0.5f) {
//		float n_dot_wi = BSDF::reflect(*this, layer_, sampler, result);
//		result.pdf *= 0.5f;
//		result.reflection *= n_dot_wi;
//	} else {
//		float n_dot_wi = BSDF::refract(*this, layer_, sampler, result);
//		result.pdf *= 0.5f;
//		result.reflection *= n_dot_wi;
//	}

	float n_dot_wi = BSDF::reflect(*this, layer_, sampler, result);
	result.reflection *= n_dot_wi;

//	float n_dot_wi = BSDF::refract(*this, layer_, sampler, result);
//	result.reflection *= n_dot_wi;
}

float3 Sample_rough::radiance() const {
	return math::float3_identity;
}

float3 Sample_rough::attenuation() const {
	return layer_.attenuation;
}

float Sample_rough::ior() const {
	return layer_.ior_i;
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

void Sample_rough::Layer::set(float3_p color, float attenuation_distance,
							  float ior, float ior_outside, float a2) {
	this->color = color;
	this->attenuation = material::Sample::attenuation(color, attenuation_distance);
	this->ior_i = ior;
	this->ior_o = ior_outside;
	this->eta_i = ior_outside / ior;
	this->eta_t = ior / ior_outside;
	this->a2 = a2;
}

float Sample_rough::BSDF::reflect(const Sample& sample, const Layer& layer,
								  sampler::Sampler& sampler, bxdf::Result& result) {
	Layer tmp = layer;

	if (!sample.same_hemisphere(sample.wo_)) {
		tmp.n *= -1.f;
		tmp.eta_i = layer.eta_t;
		tmp.eta_t = layer.eta_i;
	}

	float n_dot_wo = tmp.clamped_n_dot(sample.wo_);

	float sint2 = (tmp.eta_i * tmp.eta_i) * (1.f - n_dot_wo * n_dot_wo);

	float f;
	if (sint2 > 1.f) {
		f = 1.f;
	} else {
		float n_dot_t = std::sqrt(1.f - sint2);

		// fresnel has to be the same value that would have been computed by BRDF
		f = fresnel::dielectric(n_dot_wo, n_dot_t, tmp.eta_i, tmp.eta_t);
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
		tmp.n *= -1.f;
		tmp.eta_i = layer.eta_t;
		tmp.eta_t = layer.eta_i;
	}

	float n_dot_wo = tmp.clamped_n_dot(sample.wo_);

	float sint2 = (tmp.eta_i * tmp.eta_i) * (1.f - n_dot_wo * n_dot_wo);

	if (sint2 > 1.f) {
		result.pdf = 0.f;
		return 0.f;
	}

	float n_dot_t = std::sqrt(1.f - sint2);

	// fresnel has to be the same value that would have been computed by BRDF
	float f = fresnel::dielectric(n_dot_wo, n_dot_t, tmp.eta_i, tmp.eta_t);

	float3 f3(1.f - f);
	fresnel::Constant constant(f3);
	float n_dot_wi = ggx::Isotropic::refract(sample.wo_, n_dot_wo, n_dot_t, tmp,
											 constant, sampler, result);

	SOFT_ASSERT(testing::check(result, sample.wo_, layer));

	return n_dot_wi;
}

}}}
