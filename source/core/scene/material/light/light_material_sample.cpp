#include "light_material_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "sampler/sampler.hpp"
#include "base/math/vector3.inl"

namespace scene::material::light {

const material::Sample::Layer& Sample::base_layer() const {
	return layer_;
}

bxdf::Result Sample::evaluate(f_float3 /*wi*/) const {
	return { float3::identity(), 0.f };
}

float3 Sample::radiance() const {
	return layer_.radiance_;
}

void Sample::sample(sampler::Sampler& /*sampler*/, bxdf::Sample& result) const {
	result.reflection = float3::identity();
	result.pdf = 0.f;
}

bool Sample::is_pure_emissive() const {
	return true;
}

void Sample::Layer::set(const float3& radiance) {
	radiance_ = radiance;
}

}
