#include "ggx.hpp"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"

namespace scene { namespace material { namespace ggx {

template<typename Sample>
GGX<Sample>::GGX(const Sample& sample) : BxDF<Sample>(sample) {}

template<typename Sample>
math::float3 GGX<Sample>::evaluate(const math::float3& wi, float n_dot_wi) const {
	// Roughness zero will always have zero specular term (or worse NaN)
	if (0.f == BxDF<Sample>::sample_.a2_) {
		return math::float3::identity;
	}

	float n_dot_wo = std::max(math::dot(BxDF<Sample>::sample_.n_, BxDF<Sample>::sample_.wo_), 0.00001f);

	math::float3 h = math::normalized(BxDF<Sample>::sample_.wo_ + wi);

	float n_dot_h  = math::dot(BxDF<Sample>::sample_.n_, h);
	float wo_dot_h = math::dot(BxDF<Sample>::sample_.wo_, h);

	float d = ggx::d(n_dot_h, std::max(BxDF<Sample>::sample_.a2_, 0.0000001f));
	float g = ggx::g(n_dot_wi, n_dot_wo, BxDF<Sample>::sample_.a2_);
	math::float3 f = ggx::f(wo_dot_h, BxDF<Sample>::sample_.f0_);

	math::float3 specular = d * g * f;

	return specular;
}

template<typename Sample>
float GGX<Sample>::pdf(const math::float3& wi, float /*n_dot_wi*/) const {
	// Roughness zero will always have zero specular term (or worse NaN)
	if (0.f == BxDF<Sample>::sample_.a2_) {
		return 0.f;
	}

	math::float3 h = math::normalized(BxDF<Sample>::sample_.wo_ + wi);

	float n_dot_h  = math::dot(BxDF<Sample>::sample_.n_, h);
	float wo_dot_h = math::dot(BxDF<Sample>::sample_.wo_, h);

	float d = ggx::d(n_dot_h, std::max(BxDF<Sample>::sample_.a2_, 0.0000001f));

	return d * n_dot_h / (4.f * std::max(wo_dot_h, 0.00001f));
}

template<typename Sample>
float GGX<Sample>::importance_sample(sampler::Sampler& sampler, BxDF_result& result) const {
	math::float2 xi = sampler.generate_sample_2D();

	// For zero roughness we risk NaN if xi.y == 1: n_dot_h is always 1 anyway
	// TODO: Optimize the perfect mirror case more
	float n_dot_h = 0.f == BxDF<Sample>::sample_.a2_ ? 1.f : std::sqrt((1.f - xi.y) / ((BxDF<Sample>::sample_.a2_ - 1.f) * xi.y + 1.f));
	float sin_theta = std::sqrt(1.f - n_dot_h * n_dot_h);
	float phi = 2.f * math::Pi * xi.x;
	float sin_phi = std::sin(phi);
	float cos_phi = std::cos(phi);

	math::float3 is = math::float3(sin_theta * cos_phi, sin_theta * sin_phi, n_dot_h);
	math::float3 h = BxDF<Sample>::sample_.tangent_to_world(is);

	float wo_dot_h = math::dot(BxDF<Sample>::sample_.wo_, h);

	result.wi = math::normalized((2.f * wo_dot_h) * h - BxDF<Sample>::sample_.wo_);

	float n_dot_wi = std::max(math::dot(BxDF<Sample>::sample_.n_, result.wi),	  0.00001f);
	float n_dot_wo = std::max(math::dot(BxDF<Sample>::sample_.n_, BxDF<Sample>::sample_.wo_), 0.00001f);

	float d = ggx::d(n_dot_h, std::max(BxDF<Sample>::sample_.a2_, 0.0000001f));
	float g = ggx::g(n_dot_wi, n_dot_wo, BxDF<Sample>::sample_.a2_);
	math::float3 f = ggx::f(wo_dot_h, BxDF<Sample>::sample_.f0_);

	result.pdf = d * n_dot_h / (4.f * std::max(wo_dot_h, 0.00001f));

	math::float3 specular = d * g * f;
	result.reflection = specular;

	result.type.clear_set(0.f == BxDF<Sample>::sample_.a2_ ? BxDF_type::Specular_reflection : BxDF_type::Glossy_reflection);


//	if (math::contains_inf(specular) || math::contains_nan(specular)) {
//		std::cout << "n_dot_h == " << n_dot_h << std::endl;

//		std::cout << "a2 == " << sample_.a2_ << std::endl;
//	}

	return n_dot_wi;
}

inline math::float3 f(float wo_dot_h, const math::float3& f0) {
	return f0 + std::pow(1.f - wo_dot_h, 5.f) * math::float3(1.f - f0.x, 1.f - f0.y, 1.f - f0.z);

	// Gaussian approximation
	// return f0 + (std::exp2((-5.55473f * wo_dot_h - 6.98316f) * wo_dot_h)) * math::float3(1.f - f0.x, 1.f - f0.y, 1.f - f0.z);
}

inline float f(float wo_dot_h, float f0) {
	return f0 + std::pow(1.f - wo_dot_h, 5.f) * (1.f - f0);
}

inline float d(float n_dot_h, float a2) {
	float d = n_dot_h * n_dot_h * (a2 - 1.f) + 1.f;
	return a2 / (math::Pi * d * d);
}

inline float g(float n_dot_wi, float n_dot_wo, float a2) {
	float g_wo = n_dot_wo + std::sqrt((n_dot_wo - n_dot_wo * a2) * n_dot_wo + a2);
	float g_wi = n_dot_wi + std::sqrt((n_dot_wi - n_dot_wi * a2) * n_dot_wi + a2);
	return 1.f / (g_wo * g_wi);
}

}}}
