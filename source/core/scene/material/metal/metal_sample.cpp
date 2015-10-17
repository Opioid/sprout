#include "metal_sample.hpp"
#include "scene/material/ggx/ggx.inl"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace metal {

Sample::Sample() : ggx_(*this) {}

math::float3 Sample::evaluate(const math::float3& wi, float& pdf) const {
	float n_dot_wi = std::max(math::dot(n_, wi),  0.00001f);
	float n_dot_wo = std::max(math::dot(n_, wo_), 0.00001f);

	return n_dot_wi * ggx_.evaluate(wi, n_dot_wi, n_dot_wo, pdf);
}

math::float3 Sample::emission() const {
	return math::float3::identity;
}

math::float3 Sample::attenuation() const {
	return math::float3(100.f, 100.f, 100.f);;
}

void Sample::sample_evaluate(sampler::Sampler& sampler, BxDF_result& result) const {
	float n_dot_wi = ggx_.importance_sample(sampler, result);
	result.reflection *= n_dot_wi;
}

bool Sample::is_pure_emissive() const {
	return false;
}

bool Sample::is_translucent() const {
	return false;
}

void Sample::set(const math::float3& ior, const math::float3& absorption, float sqrt_roughness) {
	ior_ = ior;
	absorption_ = absorption;

	float roughness = sqrt_roughness * sqrt_roughness;
	a2_ = roughness * roughness;
}

}}}

