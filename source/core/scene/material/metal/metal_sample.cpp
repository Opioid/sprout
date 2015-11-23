#include "metal_sample.hpp"
#include "scene/material/ggx/ggx.inl"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace metal {

math::float3 Sample_iso::evaluate(const math::float3& wi, float& pdf) const {
	float n_dot_wi = std::max(math::dot(n_, wi),  0.00001f);
	float n_dot_wo = std::max(math::dot(n_, wo_), 0.00001f);

	return n_dot_wi * ggx_.evaluate(*this, wi, n_dot_wi, n_dot_wo, pdf);
}

math::float3 Sample_iso::emission() const {
	return math::float3::identity;
}

math::float3 Sample_iso::attenuation() const {
	return math::float3(100.f, 100.f, 100.f);;
}

void Sample_iso::sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const {
	float n_dot_wo = clamped_n_dot_wo();
	float n_dot_wi = ggx_.importance_sample(*this, sampler, n_dot_wo, result);
	result.reflection *= n_dot_wi;
}

bool Sample_iso::is_pure_emissive() const {
	return false;
}

bool Sample_iso::is_translucent() const {
	return false;
}

void Sample_iso::set(const math::float3& ior, const math::float3& absorption, float sqrt_roughness) {
	ior_ = ior;
	absorption_ = absorption;

	float roughness = sqrt_roughness * sqrt_roughness;
	a2_ = roughness * roughness;
}

math::float3 Sample_aniso::evaluate(const math::float3& wi, float& pdf) const {
	float n_dot_wi = std::max(math::dot(n_, wi),  0.00001f);
	float n_dot_wo = std::max(math::dot(n_, wo_), 0.00001f);

	return n_dot_wi * ggx_.evaluate(*this, wi, n_dot_wi, n_dot_wo, pdf);
}

math::float3 Sample_aniso::emission() const {
	return math::float3::identity;
}

math::float3 Sample_aniso::attenuation() const {
	return math::float3(100.f, 100.f, 100.f);;
}

void Sample_aniso::sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const {
	float n_dot_wo = clamped_n_dot_wo();
	float n_dot_wi = ggx_.importance_sample(*this, sampler, n_dot_wo, result);
	result.reflection *= n_dot_wi;
}

bool Sample_aniso::is_pure_emissive() const {
	return false;
}

bool Sample_aniso::is_translucent() const {
	return false;
}

void Sample_aniso::set(const math::float3& ior, const math::float3& absorption,
					   math::float2 direction, math::float2 sqrt_roughness) {
	ior_ = ior;
	absorption_ = absorption;
	direction_  = direction;

	a_ = sqrt_roughness * sqrt_roughness;

	a2_ = a_.x * a_.x;
}

}}}

