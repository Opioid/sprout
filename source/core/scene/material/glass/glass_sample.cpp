#include "glass_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"

#include "scene/material/material_test.hpp"
#include "base/debug/assert.hpp"

namespace scene { namespace material { namespace glass {

const material::Sample::Layer& Sample::base_layer() const {
	return layer_;
}

float3 Sample::evaluate(const float3& /*wi*/, float& pdf) const {
	pdf = 0.f;
	return float3::identity();
}

float3 Sample::radiance() const {
	return float3::identity();
}

float3 Sample::attenuation() const {
	return layer_.attenuation_;
}

float Sample::ior() const {
	return layer_.ior_;
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Result& result) const {
	float p = sampler.generate_sample_1D();

	if (p < 0.5f) {
		BSDF::reflect(*this, layer_, sampler, result);
		result.pdf *= 0.5f;
	} else {
		BSDF::refract(*this, layer_, sampler, result);
		result.pdf *= 0.5f;
	}
}

bool Sample::is_pure_emissive() const {
	return false;
}

bool Sample::is_transmissive() const {
	return true;
}

bool Sample::is_translucent() const {
	return false;
}

void Sample::Layer::set(const float3& refraction_color, const float3& absorption_color,
						float attenuation_distance, float ior, float ior_outside) {
	color_ = refraction_color;
	attenuation_ = material::absorption_coefficient(absorption_color, attenuation_distance);
	ior_ = ior;
	ior_outside_ = ior_outside;
}

float Sample::BSDF::reflect(const Sample& sample, const Layer& layer,
							sampler::Sampler& /*sampler*/, bxdf::Result& result) {
	float3 n = layer.n_;
	float eta_i = 1.f / layer.ior_;
	float eta_t = layer.ior_;

	if (!sample.same_hemisphere(sample.wo_)) {
		n *= -1.f;
		eta_t = eta_i;
		eta_i = layer.ior_;
	}

	const float n_dot_wo = math::saturate(math::dot(n, sample.wo_));

	const float sint2 = (eta_i * eta_i) * (1.f - n_dot_wo * n_dot_wo);

	float f;
	if (sint2 > 1.f) {
		f = 1.f;
	} else {
		const float n_dot_t = std::sqrt(1.f - sint2);

		// fresnel has to be the same value that would have been computed by BRDF
		f = fresnel::dielectric(n_dot_wo, n_dot_t, eta_i, eta_t);
	}

	result.reflection = float3(f);
	result.wi = math::normalized(2.f * n_dot_wo * n - sample.wo_);
	result.pdf = 1.f;
	result.type.clear_set(bxdf::Type::Specular_reflection);

	SOFT_ASSERT(testing::check(result, sample.wo_, layer));

	return 1.f;
}

float Sample::BSDF::refract(const Sample& sample, const Layer& layer,
							sampler::Sampler& /*sampler*/, bxdf::Result& result) {
	float3 n = layer.n_;
	float eta_i = 1.f / layer.ior_;
	float eta_t = layer.ior_;

	if (!sample.same_hemisphere(sample.wo_)) {
		n *= -1.f;
		eta_t = eta_i;
		eta_i = layer.ior_;
	}

	const float n_dot_wo = math::saturate(math::dot(n, sample.wo_));

	const float sint2 = (eta_i * eta_i) * (1.f - n_dot_wo * n_dot_wo);

	if (sint2 > 1.f) {
		result.pdf = 0.f;
		return 0.f;
	}

	const float n_dot_t = std::sqrt(1.f - sint2);

	// fresnel has to be the same value that would have been computed by BRDF
	const float f = fresnel::dielectric(n_dot_wo, n_dot_t, eta_i, eta_t);

	result.reflection = (1.f - f) * layer.color_;
	result.wi = math::normalized((eta_i * n_dot_wo - n_dot_t) * n - eta_i * sample.wo_);
	result.pdf = 1.f;
	result.type.clear_set(bxdf::Type::Specular_transmission);

	SOFT_ASSERT(testing::check(result, sample.wo_, layer));

	return 1.f;
}

}}}
