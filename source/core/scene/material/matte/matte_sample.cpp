#include "matte_sample.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/lambert/lambert.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace matte {

float3_p Sample::shading_normal() const {
	return layer_.n;
}

float3 Sample::tangent_to_world(float3_p v) const {
	return layer_.tangent_to_world(v);
}

float3 Sample::evaluate(float3_p wi, float& pdf) const {
	float n_dot_wi = layer_.clamped_n_dot(wi);

	return n_dot_wi * lambert::Isotropic::evaluate(wi, n_dot_wi, *this, layer_, pdf);
}

float3 Sample::radiance() const {
	return math::float3_identity;
}

float3 Sample::attenuation() const {
	return float3(100.f, 100.f, 100.f);
}

float Sample::ior() const {
	return 1.5f;
}

void Sample::sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const {
	float n_dot_wi = lambert::Isotropic::importance_sample(*this, layer_, sampler, result);
	result.reflection *= n_dot_wi;
}

bool Sample::is_pure_emissive() const {
	return false;
}

bool Sample::is_transmissive() const {
	return false;
}

bool Sample::is_translucent() const {
	return false;
}

void Sample::Layer::set(float3_p color) {
	this->diffuse_color = color;
}

}}}
