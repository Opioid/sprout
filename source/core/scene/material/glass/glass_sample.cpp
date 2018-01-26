#include "glass_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_attenuation.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"

#include "scene/material/material_test.hpp"
#include "base/debug/assert.hpp"

namespace scene::material::glass {

const material::Sample::Layer& Sample::base_layer() const {
	return layer_;
}

bxdf::Result Sample::evaluate(const float3& /*wi*/) const {
	return { float3::identity(), 0.f };
}

float3 Sample::absorption_coefficient() const {
	return layer_.absorption_coefficient_;
}

float Sample::ior() const {
	return layer_.ior_;
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Sample& result) const {
	const float p = sampler.generate_sample_1D();

	if (p < 0.5f) {
		BSDF::reflect(*this, layer_, sampler, result);
		result.pdf *= 0.5f;
	} else {
		BSDF::refract(*this, layer_, sampler, result);
		result.pdf *= 0.5f;
	}

	result.wavelength = 0.f;
}

bool Sample::is_transmissive() const {
	return true;
}

void Sample::Layer::set(const float3& refraction_color, const float3& absorption_color,
						float attenuation_distance, float ior, float ior_outside) {
	color_ = refraction_color;
	absorption_coefficient_ = material::extinction_coefficient(absorption_color,
															   attenuation_distance);
	ior_ = ior;
	ior_outside_ = ior_outside;
}

float Sample::BSDF::reflect(const Sample& sample, const Layer& layer,
							sampler::Sampler& /*sampler*/, bxdf::Sample& result) {
	float3 n = layer.n_;
	float eta_i = 1.f / layer.ior_;
	float eta_t = layer.ior_;

	if (!sample.same_hemisphere(sample.wo_)) {
		n = -n;
		eta_t = eta_i;
		eta_i = layer.ior_;
	}

	const float n_dot_wo = std::min(std::abs(math::dot(n, sample.wo_)), 1.f); //math::saturate(math::dot(n, sample.wo_));

	const float sint2 = (eta_i * eta_i) * (1.f - n_dot_wo * n_dot_wo);

	float f;
	if (sint2 >= 1.f) {
		f = 1.f;
	} else {
		const float n_dot_t = std::sqrt(1.f - sint2);

		// fresnel has to be the same value that would have been computed by BRDF
		f = fresnel::dielectric(n_dot_wo, n_dot_t, eta_i, eta_t);
	}

	result.reflection = float3(f);
	result.wi = math::normalize(2.f * n_dot_wo * n - sample.wo_);
	result.pdf = 1.f;
	result.type.clear(bxdf::Type::Specular_reflection);

	SOFT_ASSERT(testing::check(result, sample.wo_, layer));

	return 1.f;
}

float Sample::BSDF::refract(const Sample& sample, const Layer& layer,
							sampler::Sampler& /*sampler*/, bxdf::Sample& result) {
	float3 n = layer.n_;
	float eta_i = 1.f / layer.ior_;
	float eta_t = layer.ior_;

	if (!sample.same_hemisphere(sample.wo_)) {
		n = -n;
		eta_t = eta_i;
		eta_i = layer.ior_;
	}

	const float n_dot_wo = std::min(std::abs(math::dot(n, sample.wo_)), 1.f); //math::saturate(math::dot(n, sample.wo_));

	const float sint2 = (eta_i * eta_i) * (1.f - n_dot_wo * n_dot_wo);

	if (sint2 >= 1.f) {
		result.pdf = 0.f;
		return 0.f;
	}

	const float n_dot_t = std::sqrt(1.f - sint2);

	// fresnel has to be the same value that would have been computed by BRDF
	const float f = fresnel::dielectric(n_dot_wo, n_dot_t, eta_i, eta_t);

	result.reflection = (1.f - f) * layer.color_;
	result.wi = math::normalize((eta_i * n_dot_wo - n_dot_t) * n - eta_i * sample.wo_);
	result.pdf = 1.f;
	result.type.clear(bxdf::Type::Specular_transmission);

	SOFT_ASSERT(testing::check(result, sample.wo_, layer));

	return 1.f;
}

}
