#include "ggx.hpp"
#include "scene/material/bxdf.hpp"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"

#include <iostream>

namespace scene { namespace material { namespace ggx {

template<typename Sample>
math::float3 Isotropic_Schlick<Sample>::evaluate(const Sample& sample,
												 const math::float3& wi, float n_dot_wi, float n_dot_wo,
												 float &pdf) const {
	// Roughness zero will always have zero specular term (or worse NaN)
	if (0.f == sample.a2_) {
		pdf = 0.f;
		return math::float3::identity;
	}

	math::float3 h = math::normalized(sample.wo_ + wi);

	float n_dot_h  = math::saturate(math::dot(sample.n_, h));
	float wo_dot_h = math::clamp(math::dot(sample.wo_, h), 0.00001f, 1.f);

	float d = ggx::d(n_dot_h, std::max(sample.a2_, 0.0000001f));
	float g = ggx::g(n_dot_wi, n_dot_wo, sample.a2_);
	math::float3 f = ggx::f(wo_dot_h, sample.f0_);

	math::float3 specular = d * g * f;

//	if (math::contains_negative(specular)) {
//		std::cout << "GGX<Sample>::evaluate()" << std::endl;
//	}

//	if (math::contains_inf(specular) || math::contains_nan(specular)) {
//		std::cout << "GGX<Sample>::evaluate()" << std::endl;
//	}

	pdf = d * n_dot_h / (4.f * wo_dot_h);
	return specular;
}

template<typename Sample>
float Isotropic_Schlick<Sample>::importance_sample(const Sample& sample,
												   sampler::Sampler& sampler, float n_dot_wo,
												   bxdf::Result& result) const {
	math::float2 xi = sampler.generate_sample_2D();

	// For zero roughness we risk NaN if xi.y == 1: n_dot_h is always 1 anyway
	// TODO: Optimize the perfect mirror case more
	float n_dot_h = 0.f == sample.a2_ ? 1.f : std::sqrt((1.f - xi.y) / ((sample.a2_ - 1.f) * xi.y + 1.f));

	float sin_theta = std::sqrt(1.f - n_dot_h * n_dot_h);
	float phi = 2.f * math::Pi * xi.x;
	float sin_phi = std::sin(phi);
	float cos_phi = std::cos(phi);

	math::float3 is = math::float3(sin_theta * cos_phi, sin_theta * sin_phi, n_dot_h);
	math::float3 h = sample.tangent_to_world(is);

	float wo_dot_h = math::clamp(math::dot(sample.wo_, h), 0.00001f, 1.f);
//	float wo_dot_h = std::max(math::dot(sample.wo_, h), 0.00001f);

	math::float3 wi = math::normalized((2.f * wo_dot_h) * h - sample.wo_);

	float n_dot_wi = std::max(math::dot(sample.n_, wi),	  0.00001f);
//	float n_dot_wo = std::max(math::dot(sample.n_, BxDF<Sample>::sample_.wo_), 0.00001f);

	float d = ggx::d(n_dot_h, std::max(sample.a2_, 0.0000001f));
	float g = ggx::g(n_dot_wi, n_dot_wo, sample.a2_);
	math::float3 f = ggx::f(wo_dot_h, sample.f0_);

	result.pdf = d * n_dot_h / (4.f * wo_dot_h);

	math::float3 specular = d * g * f;
	result.reflection = specular;
	result.wi = wi;
	result.type.clear_set(0.f == sample.a2_ ? bxdf::Type::Specular_reflection
											: bxdf::Type::Glossy_reflection);

//	if (wo_dot_h > 1.f) {
//		std::cout << "GGX<Sample>::importance_sample()" << std::endl;
//	}

//	if (math::contains_negative(result.reflection)) {
//		std::cout << "GGX<Sample>::importance_sample()" << std::endl;
//		std::cout << "d: " << d << std::endl;
//		std::cout << "g: " << g << std::endl;
//		std::cout << "f: " << f << std::endl;
//		std::cout << "wo_dot_h: " << wo_dot_h << std::endl;
//	}

	return n_dot_wi;
}

math::float3 fresnel_conductor(float cos_theta_i, const math::float3& eta, const math::float3& k);

template<typename Sample>
math::float3 Isotropic_Conductor<Sample>::evaluate(const Sample& sample,
												   const math::float3& wi, float n_dot_wi, float n_dot_wo,
												   float &pdf) const {
	// Roughness zero will always have zero specular term (or worse NaN)
	if (0.f == sample.a2_) {
		pdf = 0.f;
		return math::float3::identity;
	}

	math::float3 h = math::normalized(sample.wo_ + wi);

	float n_dot_h  = math::saturate(math::dot(sample.n_, h));
	float wo_dot_h = math::clamp(math::dot(sample.wo_, h), 0.00001f, 1.f);

	float d = ggx::d(n_dot_h, std::max(sample.a2_, 0.0000001f));
	float g = ggx::g(n_dot_wi, n_dot_wo, sample.a2_);
	math::float3 f = fresnel_conductor(wo_dot_h, sample.ior_, sample.absorption_);

	math::float3 specular = d * g * f;

//	if (math::contains_negative(specular)) {
//		std::cout << "GGX<Sample>::evaluate()" << std::endl;
//	}

	pdf = d * n_dot_h / (4.f * wo_dot_h);
	return specular;
}

template<typename Sample>
float Isotropic_Conductor<Sample>::importance_sample(const Sample& sample,
													 sampler::Sampler& sampler, float n_dot_wo,
													 bxdf::Result& result) const {
	math::float2 xi = sampler.generate_sample_2D();

	// For zero roughness we risk NaN if xi.y == 1: n_dot_h is always 1 anyway
	// TODO: Optimize the perfect mirror case more
	float n_dot_h = 0.f == sample.a2_ ? 1.f : std::sqrt((1.f - xi.y) / ((sample.a2_ - 1.f) * xi.y + 1.f));

	float sin_theta = std::sqrt(1.f - n_dot_h * n_dot_h);
	float phi = 2.f * math::Pi * xi.x;
	float sin_phi = std::sin(phi);
	float cos_phi = std::cos(phi);

	math::float3 is = math::float3(sin_theta * cos_phi, sin_theta * sin_phi, n_dot_h);
	math::float3 h = sample.tangent_to_world(is);

//	float wo_dot_h = math::clamp(math::dot(sample.wo_, h), 0.00001f, 1.f);
	float wo_dot_h = std::max(math::dot(sample.wo_, h), 0.00001f);

	math::float3 wi = math::normalized((2.f * wo_dot_h) * h - sample.wo_);

	float n_dot_wi = std::max(math::dot(sample.n_, wi),	  0.00001f);
//	float n_dot_wo = std::max(math::dot(sample.n_, BxDF<Sample>::sample_.wo_), 0.00001f);

	float d = ggx::d(n_dot_h, std::max(sample.a2_, 0.0000001f));
	float g = ggx::g(n_dot_wi, n_dot_wo, sample.a2_);
	math::float3 f = fresnel_conductor(wo_dot_h, sample.ior_, sample.absorption_);

	result.pdf = d * n_dot_h / (4.f * wo_dot_h);

	math::float3 specular = d * g * f;
	result.reflection = specular;
	result.wi = wi;
	result.type.clear_set(0.f == sample.a2_ ? bxdf::Type::Specular_reflection
											: bxdf::Type::Glossy_reflection);

//	if (math::contains_negative(result.reflection)) {
//		std::cout << "GGX<Sample>::importance_sample()" << std::endl;
//		std::cout << "d: " << d << std::endl;
//		std::cout << "g: " << g << std::endl;
//		std::cout << "f: " << f << std::endl;
//		std::cout << "wo_dot_h: " << wo_dot_h << std::endl;
//	}

	return n_dot_wi;
}

template<typename Sample>
math::float3 Anisotropic_Conductor<Sample>::evaluate(const Sample& sample,
													 const math::float3& wi, float n_dot_wi, float n_dot_wo,
													 float &pdf) const {
	// Roughness zero will always have zero specular term (or worse NaN)
	if (0.f == sample.a_.x || 0.f == sample.a_.y) {
		pdf = 0.f;
		return math::float3::identity;
	}

	math::float3 h = math::normalized(sample.wo_ + wi);

	float n_dot_h  = math::saturate(math::dot(sample.n_, h));

//	float x_dot_h  = math::saturate(math::dot(sample.t_, h));
//	float y_dot_h  = math::saturate(math::dot(sample.b_, h));

	float x_dot_h  = math::dot(sample.t_, h);
	float y_dot_h  = math::dot(sample.b_, h);

	float wo_dot_h = math::clamp(math::dot(sample.wo_, h), 0.00001f, 1.f);

//	float d = ggx::d(n_dot_h, std::max(sample.a2_, 0.0000001f));
	float d = ggx::d_aniso(n_dot_h, x_dot_h, y_dot_h, sample.a_);

	float g = ggx::g(n_dot_wi, n_dot_wo, sample.a2_);
	math::float3 f = fresnel_conductor(wo_dot_h, sample.ior_, sample.absorption_);

	math::float3 specular = d * g * f;

//	if (math::contains_negative(specular)) {
//		std::cout << "GGX<Sample>::evaluate()" << std::endl;
//	}

	pdf = d * n_dot_h / (4.f * wo_dot_h);
	return specular;
}

template<typename Sample>
float Anisotropic_Conductor<Sample>::importance_sample(const Sample& sample,
													   sampler::Sampler& sampler, float n_dot_wo,
													   bxdf::Result& result) const {
	math::float2 xi = sampler.generate_sample_2D();

	float phi = 2.f * math::Pi * xi.x;
	float sin_phi = std::sin(phi);
	float cos_phi = std::cos(phi);

	float t0 = std::sqrt(xi.y / (1.f - xi.y));
	math::float3 t1 = (sample.a_.x * cos_phi * sample.t_ + sample.a_.y * sin_phi * sample.b_);

	math::float3 h = math::normalized(t0 * t1 + sample.n_);

	float x_dot_h = math::dot(sample.t_, h);
	float y_dot_h = math::dot(sample.b_, h);
	float n_dot_h = math::dot(sample.n_, h);

//	float wo_dot_h = math::clamp(math::dot(sample.wo_, h), 0.00001f, 1.f);
	float wo_dot_h = std::max(math::dot(sample.wo_, h), 0.00001f);

	math::float3 wi = math::normalized((2.f * wo_dot_h) * h - sample.wo_);

	float n_dot_wi = std::max(math::dot(sample.n_, wi),	  0.00001f);
//	float n_dot_wo = std::max(math::dot(sample.n_, BxDF<Sample>::sample_.wo_), 0.00001f);

//	float d = ggx::d(n_dot_h, std::max(sample.a2_, 0.0000001f));
	float d = ggx::d_aniso(n_dot_h, x_dot_h, y_dot_h, sample.a_);
	float g = ggx::g(n_dot_wi, n_dot_wo, sample.a2_);
	math::float3 f = fresnel_conductor(wo_dot_h, sample.ior_, sample.absorption_);

	result.pdf = d * n_dot_h / (4.f * wo_dot_h);

	math::float3 specular = d * g * f;
	result.reflection = specular;
	result.wi = wi;
	result.type.clear_set(bxdf::Type::Glossy_reflection);

//	if (math::contains_negative(result.reflection)) {
//		std::cout << "GGX<Sample>::importance_sample()" << std::endl;
//		std::cout << "d: " << d << std::endl;
//		std::cout << "g: " << g << std::endl;
//		std::cout << "f: " << f << std::endl;
//		std::cout << "wo_dot_h: " << wo_dot_h << std::endl;
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

inline float d_aniso(float n_dot_h, float x_dot_h, float y_dot_h, math::float2 a) {
	float t0 = 1.f / math::Pi;
	float t1 = 1.f / (a.x * a.y);

	float x = (x_dot_h * x_dot_h) / (a.x * a.x);
	float y = (y_dot_h * y_dot_h) / (a.y * a.y);
	float d = (x + y + n_dot_h * n_dot_h);
	float t2 = 1.f / (d * d);

	return t0 * t1 * t2;

//	float a2 = a.x * a.y;

//	float d = n_dot_h * n_dot_h * (a2 - 1.f) + 1.f;
//	return a2 / (math::Pi * d * d);

/*	float HdotX_2 = x_dot_h * x_dot_h;
	float HdotY_2 = y_dot_h * y_dot_h;
	float NdotH_2 = n_dot_h * n_dot_h;

	float ax_2 = a.x * a.x;
	float ay_2 = a.y * a.y;

	return a.x * a.y * std::pow(HdotX_2 / ax_2 + HdotY_2 / ay_2 + NdotH_2, 2.0);
	*/
}

inline float g(float n_dot_wi, float n_dot_wo, float a2) {
	float g_wo = n_dot_wo + std::sqrt((n_dot_wo - n_dot_wo * a2) * n_dot_wo + a2);
	float g_wi = n_dot_wi + std::sqrt((n_dot_wi - n_dot_wi * a2) * n_dot_wi + a2);
	return 1.f / (g_wo * g_wi);
}

inline math::float3 fresnel_conductor(float cos_theta_i, const math::float3& eta, const math::float3& k) {
	math::float3 tmp_f = (eta * eta + k * k);
	math::float3 tmp = cos_theta_i * cos_theta_i * tmp_f;

	math::float3 r_p = (tmp - (2.f * cos_theta_i * eta) + 1.f)
					 / (tmp + (2.f * cos_theta_i * eta) + 1.f);

	math::float3 r_o = (tmp_f - (2.f * cos_theta_i * eta) + cos_theta_i * cos_theta_i)
					 / (tmp_f + (2.f * cos_theta_i * eta) + cos_theta_i * cos_theta_i);

	return 0.5f * (r_p + r_o);
}

}}}
