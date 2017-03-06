#include "light_material_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "sampler/sampler.hpp"
#include "base/math/vector3.inl"

namespace scene { namespace material { namespace light {

const material::Sample::Layer& Sample::base_layer() const {
	return layer_;
}

float3 Sample::evaluate(float3_p /*wi*/, float& pdf) const {
	pdf = 0.f;
	return float3::identity();
}

float3 Sample::radiance() const {
	return layer_.radiance_;
}

float3 Sample::attenuation() const {
	return float3(100.f, 100.f, 100.f);
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

bool Sample::is_transmissive() const {
	return false;
}

bool Sample::is_translucent() const {
	return false;
}

void Sample::Layer::set(float3_p radiance) {
	radiance_ = radiance;
}

}}}
