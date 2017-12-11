#include "volumetric_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "base/math/vector3.inl"

namespace scene::material::volumetric {

const material::Sample::Layer& Sample::base_layer() const {
	return layer_;
}

bxdf::Result Sample::evaluate(const float3& wi) const {
	return { float3::identity(), 0.f };
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Sample& result) const {
	result.pdf = 0.f;
}

float Sample::ior() const {
	return 1.f;
}

void Sample::Layer::set(float anisotropy) {
	anisotropy_ = anisotropy;
}

}
