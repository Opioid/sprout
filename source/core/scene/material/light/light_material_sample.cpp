#include "light_material_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "sampler/sampler.hpp"
#include "base/math/vector3.inl"

namespace scene { namespace material { namespace light {

const material::Sample::Layer& Sample::base_layer() const {
	return layer_;
}

float3 Sample::evaluate(const float3& /*wi*/, float& pdf) const {
	pdf = 0.f;
	return float3::identity();
}

float3 Sample::radiance() const {
	return layer_.radiance_;
}

float Sample::ior() const {
	return 1.5f;
}

void Sample::sample(sampler::Sampler& /*sampler*/, bxdf::Result& result) const {
	result.reflection = float3::identity();
	result.pdf = 0.f;
}

bool Sample::is_pure_emissive() const {
	return true;
}

void Sample::Layer::set(const float3& radiance) {
	radiance_ = radiance;
}

}}}
