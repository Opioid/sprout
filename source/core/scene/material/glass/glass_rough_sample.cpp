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
	pdf = 0.f;
	return math::float3_identity;
}

float3 Sample_rough::radiance() const {
	return math::float3_identity;
}

float3 Sample_rough::attenuation() const {
	return layer_.attenuation;
}

float Sample_rough::ior() const {
	return layer_.ior;
}

void Sample_rough::sample(sampler::Sampler& sampler, bxdf::Result& result) const {
	float p = sampler.generate_sample_1D();

	if (p < 0.5f) {
		float n_dot_wi = BRDF::sample(*this, layer_, sampler, result);
		result.pdf *= 0.5f;
		result.reflection *= n_dot_wi;
	} else {
		BTDF::sample(*this, layer_, sampler, result);
		result.pdf *= 0.5f;
	}

//	float n_dot_wi = BRDF::sample(*this, layer_, sampler, result);
//	result.reflection *= n_dot_wi;
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
	this->ior = ior;
	this->ior_outside = ior_outside;
	this->a2 = a2;
}

float Sample_rough::BRDF::sample(const Sample& sample, const Layer& layer,
								 sampler::Sampler& sampler, bxdf::Result& result) {
	float3 n = layer.n;
	float eta_i = 1.f / layer.ior;
	float eta_t = layer.ior;

	if (!sample.same_hemisphere(sample.wo_)) {
		n *= -1.f;
		eta_t = eta_i;
		eta_i = layer.ior;
	}

	float n_dot_wo = math::saturate(math::dot(n, sample.wo_));

	float sint2 = (eta_i * eta_i) * (1.f - n_dot_wo * n_dot_wo);

	float f;
	if (sint2 > 1.f) {
		f = 1.f;
	} else {
		float n_dot_t = std::sqrt(1.f - sint2);

		// fresnel has to be the same value that would have been computed by BRDF
		f = fresnel::dielectric(n_dot_wo, n_dot_t, eta_i, eta_t);
	}

	float3 f3(f);
	fresnel::Constant constant(f3);
	float n_dot_wi = ggx::Isotropic::reflect(sample.wo_, n_dot_wo, layer,
											 constant, sampler, result);

	SOFT_ASSERT(testing::check(result, sample.wo_, layer));

	return n_dot_wi;
}

float Sample_rough::BTDF::sample(const Sample& sample, const Layer& layer,
								 sampler::Sampler& /*sampler*/, bxdf::Result& result) {
	float3 n = layer.n;
	float eta_i = 1.f / layer.ior;
	float eta_t = layer.ior;

	if (!sample.same_hemisphere(sample.wo_)) {
		n *= -1.f;
		eta_t = eta_i;
		eta_i = layer.ior;
	}

	float n_dot_wo = math::saturate(math::dot(n, sample.wo_));

	float sint2 = (eta_i * eta_i) * (1.f - n_dot_wo * n_dot_wo);

	if (sint2 > 1.f) {
		result.pdf = 0.f;
		return 0.f;
	}

	float n_dot_t = std::sqrt(1.f - sint2);

	// fresnel has to be the same value that would have been computed by BRDF
	float f = fresnel::dielectric(n_dot_wo, n_dot_t, eta_i, eta_t);

	result.reflection = (1.f - f) * layer.color;
	result.wi = math::normalized((eta_i * n_dot_wo - n_dot_t) * n - eta_i * sample.wo_);
	result.pdf = 1.f;
	result.type.clear_set(bxdf::Type::Specular_transmission);

	SOFT_ASSERT(testing::check(result, sample.wo_, layer));

	return 1.f;
}

}}}
