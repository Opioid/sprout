#include "metal_sample.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/ggx/ggx.inl"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace metal {

math::float3 Sample_isotropic::evaluate(math::pfloat3 wi, float& pdf) const {
	float n_dot_wi = std::max(math::dot(n_, wi),  0.00001f);
	float n_dot_wo = std::max(math::dot(n_, wo_), 0.00001f);

	return n_dot_wi * ggx::Conductor_isotropic::evaluate(*this, wi, n_dot_wi, n_dot_wo, pdf);
}

math::float3 Sample_isotropic::emission() const {
	return math::float3_identity;
}

math::float3 Sample_isotropic::attenuation() const {
	return math::float3(100.f, 100.f, 100.f);
}

float Sample_isotropic::ior() const {
	return 1.5f;
}

void Sample_isotropic::sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const {
	float n_dot_wo = clamped_n_dot_wo();
	float n_dot_wi = ggx::Conductor_isotropic::importance_sample(*this, sampler, n_dot_wo, result);
	result.reflection *= n_dot_wi;
}

bool Sample_isotropic::is_pure_emissive() const {
	return false;
}

bool Sample_isotropic::is_transmissive() const {
	return false;
}

bool Sample_isotropic::is_translucent() const {
	return false;
}

void Sample_isotropic::set(const math::float3& ior,
						   const math::float3& absorption,
						   float roughness) {
	ior_ = ior;
	absorption_ = absorption;

	float a = roughness * roughness;
	a2_ = a * a;
}

math::float3 Sample_anisotropic::evaluate(math::pfloat3 wi, float& pdf) const {
	float n_dot_wi = std::max(math::dot(n_, wi),  0.00001f);
	float n_dot_wo = std::max(math::dot(n_, wo_), 0.00001f);

	return n_dot_wi * ggx::Conductor_anisotropic::evaluate(*this, wi, n_dot_wi, n_dot_wo, pdf);
}

math::float3 Sample_anisotropic::emission() const {
	return math::float3_identity;
}

math::float3 Sample_anisotropic::attenuation() const {
	return math::float3(100.f, 100.f, 100.f);
}

float Sample_anisotropic::ior() const {
	return 1.5f;
}

void Sample_anisotropic::sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const {
	float n_dot_wo = clamped_n_dot_wo();
	float n_dot_wi = ggx::Conductor_anisotropic::importance_sample(*this, sampler,
																   n_dot_wo, result);
	result.reflection *= n_dot_wi;
}

bool Sample_anisotropic::is_pure_emissive() const {
	return false;
}

bool Sample_anisotropic::is_transmissive() const {
	return false;
}

bool Sample_anisotropic::is_translucent() const {
	return false;
}

void Sample_anisotropic::set(const math::float3& ior, const math::float3& absorption,
							 math::float2 roughness) {
	ior_ = ior;
	absorption_ = absorption;

	a_ = roughness * roughness;
	a2_ = a_ * a_;

	axy_ = a_.x * a_.y;
}

}}}
