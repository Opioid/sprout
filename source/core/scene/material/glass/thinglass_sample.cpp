#include "thinglass_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "rendering/integrator/surface/integrator_helper.hpp"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"

#include "scene/material/material_test.hpp"
#include "base/debug/assert.hpp"

namespace scene { namespace material { namespace glass {

const material::Sample::Layer& Sample_thin::base_layer() const {
	return layer_;
}

float3 Sample_thin::evaluate(float3_p /*wi*/, float& pdf) const {
	pdf = 0.f;
	return float3::identity();
}

float3 Sample_thin::radiance() const {
	return float3::identity();
}

float3 Sample_thin::attenuation() const {
	return layer_.attenuation_;
}

float Sample_thin::ior() const {
	return layer_.ior_;
}

void Sample_thin::sample(sampler::Sampler& sampler, bxdf::Result& result) const {
	float p = sampler.generate_sample_1D();

	if (p < 0.5f) {
		BSDF::reflect(*this, layer_, sampler, result);
		result.pdf *= 0.5f;
	} else {
		BSDF::refract(*this, layer_, sampler, result);
		result.pdf *= 0.5f;
	}
}

bool Sample_thin::is_pure_emissive() const {
	return false;
}

bool Sample_thin::is_transmissive() const {
	return true;
}

bool Sample_thin::is_translucent() const {
	return true;
}

void Sample_thin::Layer::set(float3_p refraction_color, float3_p absorption_color,
							 float attenuation_distance, float ior, float ior_outside,
							 float thickness) {
	color_ = refraction_color;
	attenuation_ = material::Sample::attenuation(absorption_color, attenuation_distance);
	ior_ = ior;
	ior_outside_ = ior_outside;
	thickness_ = thickness;
}

float Sample_thin::BSDF::reflect(const Sample_thin& sample, const Layer& layer,
								 sampler::Sampler& /*sampler*/, bxdf::Result& result) {
	float3 n = layer.n_;
	float eta_i = 1.f / layer.ior_;
	float eta_t = layer.ior_;

	if (!sample.same_hemisphere(sample.wo_)) {
		n *= -1.f;
		eta_t = eta_i;
		eta_i = layer.ior_;
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

	result.reflection = float3(f);
	result.wi = math::normalized(2.f * n_dot_wo * n - sample.wo_);
	result.pdf = 1.f;
	result.type.clear_set(bxdf::Type::Specular_reflection);

	SOFT_ASSERT(testing::check(result, sample.wo_, layer));

	return 1.f;
}

float Sample_thin::BSDF::refract(const Sample_thin& sample, const Layer& layer,
								 sampler::Sampler& /*sampler*/, bxdf::Result& result) {
	float3 n = layer.n_;
	float eta_i = 1.f / layer.ior_;
	float eta_t = layer.ior_;

	if (!sample.same_hemisphere(sample.wo_)) {
		n *= -1.f;
		eta_t = eta_i;
		eta_i = layer.ior_;
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

	float n_dot_wi = layer.clamped_n_dot(sample.wo_);
	float approximated_distance = layer.thickness_ / n_dot_wi;
	float3 attenuation = rendering::attenuation(approximated_distance, layer.attenuation_);

	result.reflection = (1.f - f) * layer.color_ * attenuation;
	result.wi = -sample.wo_;
	result.pdf = 1.f;
	// The integrator should not handle this like a proper transmission.
	result.type.clear_set(bxdf::Type::Specular_reflection);

	SOFT_ASSERT(testing::check(result, sample.wo_, layer));

	return 1.f;
}

}}}
