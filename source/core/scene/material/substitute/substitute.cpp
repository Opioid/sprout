#include "substitute.hpp"
#include "scene/material/lambert/lambert.inl"
#include "scene/material/ggx/ggx.inl"
#include "sampler/sampler.hpp"
#include "base/math/sampling.inl"
#include "base/math/vector.inl"
#include "base/math/math.hpp"
#include <iostream>

namespace scene { namespace material { namespace substitute {

GGX::GGX(const Sample& sample) : BxDF(sample) {}

math::float3 GGX::evaluate(const math::float3& wi, float n_dot_wi) const {
	// Roughness zero will always have zero specular term (or worse NaN)
	if (0.f == sample_.a2_) {
		return math::float3::identity;
	}

	float n_dot_wo = std::max(math::dot(sample_.n_, sample_.wo_), 0.00001f);

	math::float3 h = math::normalized(sample_.wo_ + wi);

	float n_dot_h  = math::dot(sample_.n_, h);
	float wo_dot_h = math::dot(sample_.wo_, h);

	math::float3 specular = ggx::d(n_dot_h, sample_.a2_) * ggx::g(n_dot_wi, n_dot_wo, sample_.a2_) * ggx::f(wo_dot_h, sample_.f0_);

	return specular;
}

float GGX::pdf(const math::float3& wi, float /*n_dot_wi*/) const {
	// Roughness zero will always have zero specular term (or worse NaN)
	if (0.f == sample_.a2_) {
		return 0.f;
	}

	math::float3 h = math::normalized(sample_.wo_ + wi);

	float n_dot_h  = math::dot(sample_.n_, h);
	float wo_dot_h = math::dot(sample_.wo_, h);

	float d = ggx::d(n_dot_h, sample_.a2_);

	return d * n_dot_h / (4.f * std::max(wo_dot_h, 0.00001f));
}

float GGX::importance_sample(sampler::Sampler& sampler, BxDF_result& result) const {
	math::float2 xi = sampler.generate_sample_2D();

	// For zero roughness we risk NaN if xi.y == 1: n_dot_h is always 1 anyway
	// TODO: Optimize the perfect mirror case more
	float n_dot_h = 0.f == sample_.a2_ ? 1.f : std::sqrt((1.f - xi.y) / ((sample_.a2_ - 1.f) * xi.y + 1.f));
	float sin_theta = std::sqrt(1.f - n_dot_h * n_dot_h);
	float phi = 2.f * math::Pi * xi.x;
	float sin_phi = std::sin(phi);
	float cos_phi = std::cos(phi);

	math::float3 is = math::float3(sin_theta * cos_phi, sin_theta * sin_phi, n_dot_h);
	math::float3 h = sample_.tangent_to_world(is);

	float wo_dot_h = math::dot(sample_.wo_, h);

	result.wi = math::normalized((2.f * wo_dot_h) * h - sample_.wo_);

	float n_dot_wi = std::max(math::dot(sample_.n_, result.wi),	  0.00001f);
	float n_dot_wo = std::max(math::dot(sample_.n_, sample_.wo_), 0.00001f);

	float d = ggx::d(n_dot_h, sample_.a2_);
	float g = ggx::g(n_dot_wi, n_dot_wo, sample_.a2_);
	math::float3 f = ggx::f(wo_dot_h, sample_.f0_);

	// Hackedy hack hack
	if (sample_.a2_ < 0.0000001f) {
		d = 1.f;
	}

	result.pdf = d * n_dot_h / (4.f * std::max(wo_dot_h, 0.00001f));

	math::float3 specular = d * g * f;
	result.reflection = specular;

	result.type.clear_set(0.f == sample_.a2_ ? BxDF_type::Specular_reflection : BxDF_type::Glossy_reflection);


//	if (math::contains_inf(specular) || math::contains_nan(specular)) {
//		std::cout << "n_dot_h == " << n_dot_h << std::endl;

//		std::cout << "a2 == " << sample_.a2_ << std::endl;
//	}

	return n_dot_wi;
}

Sample::Sample() : lambert_(*this), ggx_(*this) {}

math::float3 Sample::evaluate(const math::float3& wi, float& pdf) const {
	float n_dot_wi = std::max(math::dot(n_, wi),  0.00001f);

	// Roughness zero will always have zero specular term (or worse NaN)
	if (0.f == a2_) {
		pdf = n_dot_wi * math::Pi_inv;
		return n_dot_wi * math::Pi_inv * diffuse_color_;
	}

	float n_dot_wo = std::max(math::dot(n_, wo_), 0.00001f);

	math::float3 h = math::normalized(wo_ + wi);

	float n_dot_h  = math::dot(n_, h);
	float wo_dot_h = math::dot(wo_, h);

	float d = ggx::d(n_dot_h, a2_);

	math::float3 specular = d * ggx::g(n_dot_wi, n_dot_wo, a2_) * ggx::f(wo_dot_h, f0_);

	pdf = 0.5f * (d * n_dot_h / (4.f * std::max(wo_dot_h, 0.00001f)) + (n_dot_wi * math::Pi_inv));

	return n_dot_wi * ((math::Pi_inv * diffuse_color_) + specular);
}

math::float3 Sample::emission() const {
	return emission_;
}

math::float3 Sample::attenuation() const {
	return math::float3(100.f, 100.f, 100.f);
}

void Sample::sample_evaluate(sampler::Sampler& sampler, BxDF_result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	if (1.f == metallic_) {
		float n_dot_wi = ggx_.importance_sample(sampler, result);
		result.reflection *= n_dot_wi;
	} else {
		float p = sampler.generate_sample_1D();

//		if (p < 0.5f) {
//			float n_dot_wi = lambert_.importance_sample(sampler, result);
//			result.pdf *= 0.5f;
//			result.reflection *= n_dot_wi;
//		} else {
//			float n_dot_wi = ggx_.importance_sample(sampler, result);
//			result.pdf *= 0.5f;
//			result.reflection *= n_dot_wi;
//		}

		if (p < 0.5f) {
			float n_dot_wi = lambert_.importance_sample(sampler, result);
			result.pdf = 0.5f * (result.pdf + ggx_.pdf(result.wi, n_dot_wi));
			result.reflection = n_dot_wi * (result.reflection + ggx_.evaluate(result.wi, n_dot_wi));
		} else {
			float n_dot_wi = ggx_.importance_sample(sampler, result);
			result.pdf = 0.5f * (result.pdf + lambert_.pdf(result.wi, n_dot_wi));
			result.reflection = n_dot_wi * (result.reflection + lambert_.evaluate(result.wi, n_dot_wi));
		}
	}
}

bool Sample::is_pure_emissive() const {
	return false;
}

void Sample::set(const math::float3& color, float roughness, float metallic) {
	diffuse_color_ = (1.f - metallic) * color;
	f0_ = math::lerp(math::float3(0.03f, 0.03f, 0.03f), color, metallic);
	emission_ = math::float3::identity;

	float a = roughness * roughness;
	a2_ = a * a;

	metallic_ = metallic;
}

void Sample::set(const math::float3& color, const math::float3& emission, float roughness, float metallic) {
	diffuse_color_ = (1.f - metallic) * color;
	f0_ = math::lerp(math::float3(0.03f, 0.03f, 0.03f), color, metallic);
	emission_ = emission;

	float a = roughness * roughness;
	a2_ = a * a;

	metallic_ = metallic;
}

Substitute::Substitute(Sample_cache<Sample>& cache, std::shared_ptr<image::Image> mask) : Material(cache, mask) {}

math::float3 Substitute::sample_emission(math::float2 /*uv*/, const image::sampler::Sampler_2D& /*sampler*/) const {
	return math::float3::identity;
}

math::float3 Substitute::average_emission() const {
	return math::float3::identity;
}

const image::Texture_2D* Substitute::emission_map() const {
	return nullptr;
}

}}}
