#include "substitute.hpp"
#include "scene/material/ggx/ggx.inl"
#include "sampler/sampler.hpp"
#include "base/math/sampling.hpp"
#include "base/math/vector.inl"
#include "base/math/math.hpp"

namespace scene { namespace material { namespace substitute {

Lambert::Lambert(const Sample& sample) : sample_(sample) {}

math::float3 Lambert::evaluate(const math::float3& wi) const {
	return math::float3::identity;
}

math::float3 Lambert::importance_sample(sampler::Sampler& sampler, math::float3& wi, float& pdf) const {
	math::float2 s2d = sampler.generate_sample2d(0);

	math::float3 is = math::sample_hemisphere_cosine(s2d);
	wi = math::normalized(sample_.tangent_to_world(is));

	pdf = 1.f;

	return sample_.diffuse_color_;
}

GGX::GGX(const Sample& sample) : sample_(sample) {}

math::float3 GGX::evaluate(const math::float3& wi) const {
	return math::float3::identity;
}

math::float3 GGX::importance_sample(sampler::Sampler& sampler, math::float3& wi, float& pdf) const {
	math::float2 xi = sampler.generate_sample2d(0);

	float n_dot_h = std::sqrt((1.f - xi.y) / ((sample_.a2_ - 1.f) * xi.y + 1.f));
	float sin_theta = std::sqrt(1.f - n_dot_h * n_dot_h);
	float phi = 2.f * math::Pi * xi.x;
	float sin_phi = std::sin(phi);
	float cos_phi = std::cos(phi);

	math::float3 is = math::float3(sin_theta * cos_phi, sin_theta * sin_phi, n_dot_h);
	math::float3 h = sample_.tangent_to_world(is);

	float wo_dot_h = math::dot(sample_.wo_, h);

	wi = math::normalized((2.f * wo_dot_h) * h - sample_.wo_);

	float n_dot_wi = std::max(math::dot(sample_.n_, wi),  0.00001f);
	float n_dot_wo = std::max(math::dot(sample_.n_, sample_.wo_), 0.00001f);

	math::float3 f = ggx::f(wo_dot_h, sample_.f0_);
	float g = ggx::g(n_dot_wi, n_dot_wo, sample_.a2_);

	pdf = n_dot_h / (4.f * wo_dot_h);

	math::float3 specular = g * f;
	return n_dot_wi * specular;
}

Sample::Sample() : lambert_(*this), ggx_(*this) {}

math::float3 Sample::evaluate(const math::float3& wi) const {
	float n_dot_wi = std::max(math::dot(n_, wi),  0.00001f);
	float n_dot_wo = std::max(math::dot(n_, wo_), 0.00001f);

	math::float3 h = math::normalized(wo_ + wi);

	float n_dot_h  = math::dot(n_, h);
	float wo_dot_h = math::dot(wo_, h);

	math::float3 specular = ggx::d(n_dot_h, a2_) * ggx::g(n_dot_wi, n_dot_wo, a2_) * ggx::f(wo_dot_h, f0_);

	return n_dot_wi * ((math::Pi_inv * diffuse_color_) + specular);
}

math::float3 Sample::sample_evaluate(sampler::Sampler& sampler, math::float3& wi, float& pdf) const {
	if (!same_hemisphere(wo_)) {
		pdf = 0.f;
		return math::float3::identity;
	}

	if (1.f == metallic_) {
		return ggx_.importance_sample(sampler, wi, pdf);
	} else {
		float p = sampler.generate_sample1d(0);

		if (p < 0.5f) {
			math::float3 r = lambert_.importance_sample(sampler, wi, pdf);
			pdf *= 0.5f;
			return r;
		} else {
			math::float3 r = ggx_.importance_sample(sampler, wi, pdf);
			pdf *= 0.5f;
			return r;
		}
	}
}

void Sample::set(const math::float3& color, float roughness, float metallic) {
	diffuse_color_ = (1.f - metallic) * color;
	f0_ = math::lerp(math::float3(0.03f, 0.03f, 0.03f), color, metallic);

	float a = roughness * roughness;
	a2_ = a * a;

	metallic_ = metallic;
}

Substitute::Substitute(Sample_cache<Sample>& cache) : Material(cache) {}

}}}
