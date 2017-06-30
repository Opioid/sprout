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

float3 Sample_rough::evaluate(const float3& wi, float& pdf) const {
	if (!same_hemisphere(wo_)) {
		// only handling reflection for now
		pdf = 0.f;
		return float3::identity();
	}

	const float n_dot_wi = layer_.clamped_n_dot(wi);
	const float n_dot_wo = layer_.clamped_n_dot(wo_);

	const float sint2 = (layer_.eta_i_ * layer_.eta_i_) * (1.f - n_dot_wo * n_dot_wo);

	float f;
	if (sint2 > 1.f) {
		f = 1.f;
	} else {
		const float n_dot_t = std::sqrt(1.f - sint2);

		// fresnel has to be the same value that would have been computed by BRDF
		f = fresnel::dielectric(n_dot_wo, n_dot_t, layer_.eta_i_, layer_.eta_t_);
	}

	const float3 h = math::normalized(wo_ + wi);
	const float wo_dot_h = clamped_dot(wo_, h);

	const float n_dot_h = math::saturate(math::dot(layer_.n_, h));

	const fresnel::Constant constant(f);
	const float3 reflection = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h,
														 layer_, constant, pdf);

	return n_dot_wi * reflection;

}

void Sample_rough::sample(sampler::Sampler& sampler, bxdf::Result& result) const {
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

void Sample_rough::Layer::set(const float3& refraction_color, const float3& absorption_color,
							  float attenuation_distance, float ior,
							  float ior_outside, float a2) {
	color_ = refraction_color;
	attenuation_ = material::absorption_coefficient(absorption_color, attenuation_distance);
	ior_i_ = ior;
	ior_o_ = ior_outside;
	eta_i_ = ior_outside / ior;
	eta_t_ = ior / ior_outside;
	a2_ = a2;
}

float Sample_rough::BSDF::reflect(const Sample& sample, const Layer& layer,
								  sampler::Sampler& sampler, bxdf::Result& result) {
	Layer tmp = layer;

	if (!sample.same_hemisphere(sample.wo())) {
		tmp.n_	  *= -1.f;
		tmp.ior_i_ = layer.ior_o_;
		tmp.ior_o_ = layer.ior_i_;
		tmp.eta_i_ = layer.eta_t_;
		tmp.eta_t_ = layer.eta_i_;
	}

	const float n_dot_wo = tmp.clamped_n_dot(sample.wo());

	const float sint2 = (tmp.eta_i_ * tmp.eta_i_) * (1.f - n_dot_wo * n_dot_wo);

	float f;
	if (sint2 > 1.f) {
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
								  sampler::Sampler& sampler, bxdf::Result& result) {
	Layer tmp = layer;

	if (!sample.same_hemisphere(sample.wo())) {
		tmp.n_	  *= -1.f;
		tmp.ior_i_ = layer.ior_o_;
		tmp.ior_o_ = layer.ior_i_;
		tmp.eta_i_ = layer.eta_t_;
		tmp.eta_t_ = layer.eta_i_;
	}

	const float n_dot_wo = tmp.clamped_n_dot(sample.wo());

	const float sint2 = (tmp.eta_i_ * tmp.eta_i_) * (1.f - n_dot_wo * n_dot_wo);

	if (sint2 > 1.f) {
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

}}}
