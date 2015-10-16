#include "metal_sample.hpp"
#include "scene/material/ggx/ggx.inl"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace metal {

Sample::Sample() : ggx_(*this) {}

math::float3 Sample::evaluate(const math::float3& /*wi*/, float& pdf) const {
	pdf = 0.f;
	return math::float3::identity;
}

math::float3 Sample::emission() const {
	return math::float3::identity;
}

math::float3 Sample::attenuation() const {
	return math::float3(100.f, 100.f, 100.f);;
}

void Sample::sample_evaluate(sampler::Sampler& sampler, BxDF_result& result) const {

}

bool Sample::is_pure_emissive() const {
	return false;
}

bool Sample::is_translucent() const {
	return false;
}

void Sample::set(const math::float3& color, float roughness, float ior) {
	color_ = color;
	roughness_ = roughness;
	ior_ = ior;
}

}}}

