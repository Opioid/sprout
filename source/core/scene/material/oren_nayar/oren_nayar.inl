#include "oren_nayar.hpp"
#include "scene/material/bxdf.hpp"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace material { namespace oren_nayar {

template<typename Sample>
math::float3 Isotropic::evaluate(math::pfloat3 wi, float n_dot_wi, float n_dot_wo,
								 const Sample& sample, float& pdf) {
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
}

template<typename Sample>
float Isotropic::importance_sample(float n_dot_wo, const Sample& sample,
								   sampler::Sampler& sampler, bxdf::Result& result) {
	math::float2 s2d = sampler.generate_sample_2D();

	math::float3 is = math::sample_hemisphere_cosine(s2d);
	math::float3 wi = math::normalized(sample.tangent_to_world(is));

	float n_dot_wi = std::max(math::dot(sample.n_, wi), 0.00001f);
//	float n_dot_wi = std::abs(math::dot(sample.n_, wi));

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

	result.pdf = n_dot_wi * math::Pi_inv;
	result.reflection = math::Pi_inv * (a + b * s * t) * sample.diffuse_color_;
	result.wi = wi;
	result.type.clear_set(bxdf::Type::Diffuse_reflection);

	return n_dot_wi;
}

}}}

