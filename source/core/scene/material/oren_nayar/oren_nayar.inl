#include "oren_nayar.hpp"
#include "scene/material/bxdf.hpp"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace material { namespace oren_nayar {

template<typename Sample>
math::float3 Oren_nayar<Sample>::evaluate(const Sample& sample,
										  const math::float3& wi, float n_dot_wi, float n_dot_wo,
										  float& pdf) const {
/*
	float roughness = 1.f;
	float roughness_square = roughness * roughness;

	float alpha = std::max(std::acos(n_dot_wo), std::acos(n_dot_wi));
	float beta  = std::min(std::acos(n_dot_wo), std::acos(n_dot_wi));
	float gamma = math::dot( BxDF<Sample>::sample_.wo_ - n_dot_wo *  BxDF<Sample>::sample_.n_, wi - n_dot_wi *  BxDF<Sample>::sample_.n_);

	float c1 = 1.f - 0.5f * (roughness_square / (roughness_square + 0.33f));
	float c2 = 0.45f * (roughness_square / (roughness_square + 0.09));



	float wi_dot_wo = math::dot(wi, BxDF<Sample>::sample_.wo_);

	float s = wi_dot_wo - n_dot_wi * n_dot_wo;

	float t;
	if (s > 0.f) {
		t = std::min(1.f, n_dot_wi / n_dot_wo);
	} else {
		t = n_dot_wi;
	}


	float sin_alpha = std::sin(alpha);

	if (gamma > 0.f) {
		c2 *= sin_alpha;
	} else {
		c2 *= sin_alpha - std::pow((2.f * beta) / math::Pi, 3.f);
	}

	float c3 = 1.f / 8.f;
	c3 *= roughness_square / (roughness_square + 0.09f);
	c3 *= std::pow((4.f * alpha * beta) / (math::Pi * math::Pi), 2.f);

	float tan_beta = std::tan(beta);
	float tan_alpha_beta_2 = std::tan((alpha + beta) / 2.f);

	float a  = gamma * c2 * tan_beta;
	float b = (1.f - std::abs(gamma)) * c3 * tan_alpha_beta_2;

	return math::Pi_inv * (c1 + a + b) *  BxDF<Sample>::sample_.diffuse_color_;
*/
	float wi_dot_wo = math::dot(wi, sample.wo_);

	float s = wi_dot_wo - n_dot_wi * n_dot_wo;

	float t;
	if (s >= 0.f) {
		t = std::min(1.f, n_dot_wi / n_dot_wo);
	} else {
		t = n_dot_wi;
	}

	float a2 = sample.a2_;
	float a = 1.f - 0.5f * (a2 / (a2 + 0.33f));
	float b = 0.45f * (a2 / (a2 + 0.09f));

	pdf = n_dot_wi * math::Pi_inv;
	return math::Pi_inv * (a + b * s * t) * sample.diffuse_color_;

//	if (math::contains_negative(math::Pi_inv * (a + b * s * t) * BxDF<Sample>::sample_.diffuse_color_)) {
//		std::cout << "Oren_nayar<Sample>::evaluate()" << std::endl;
//	}

	// http://mimosa-pudica.net/improved-oren-nayar.html
/*
	float wi_dot_wo = math::dot(wi, BxDF<Sample>::sample_.wo_);

	float s = wi_dot_wo - n_dot_wi * n_dot_wo;

//	float t;
//	if (s <= 0.f) {
//		t = 1.f;
//	} else {
//		t = std::max(n_dot_wi, n_dot_wo);
//	}

	float t;
	if (s >= 0.f) {
		t = std::min(1.f, n_dot_wi / n_dot_wo);
	} else {
		t = n_dot_wi;
	}

	float sigma = 1.f;
	float a = 1.f / (math::Pi + (math::Pi_div_2 - 2.f / 3.f) * sigma);
	float b = sigma * a;

//	return (a + b * (s / t)) * BxDF<Sample>::sample_.diffuse_color_;
	return (a + b * s * t) * BxDF<Sample>::sample_.diffuse_color_;
*/
}

template<typename Sample>
float Oren_nayar<Sample>::importance_sample(const Sample& sample,
											sampler::Sampler& sampler, float n_dot_wo,
											bxdf::Result& result) const {
	math::float2 s2d = sampler.generate_sample_2D();

	math::float3 is = math::sample_hemisphere_cosine(s2d);
	math::float3 wi = math::normalized(sample.tangent_to_world(is));

	float n_dot_wi = std::max(math::dot(sample.n_, wi), 0.00001f);
	result.pdf = n_dot_wi * math::Pi_inv;

	float wi_dot_wo = math::dot(wi, sample.wo_);

	float s = wi_dot_wo - n_dot_wi * n_dot_wo;

	float t;
	if (s >= 0.f) {
		t = std::min(1.f, n_dot_wi / n_dot_wo);
	} else {
		t = n_dot_wi;
	}

	float a2 = sample.a2_;
	float a = 1.f - 0.5f * (a2 / (a2 + 0.33f));
	float b = 0.45f * (a2 / (a2 + 0.09f));

	result.reflection = math::Pi_inv * (a + b * s * t) * sample.diffuse_color_;
	result.wi = wi;
	result.type.clear_set(bxdf::Type::Diffuse_reflection);

//	if (math::contains_negative(result.reflection)) {
//		std::cout << "Oren_nayar<Sample>::importance_sample()" << std::endl;
//	}

	return n_dot_wi;
}

}}}

