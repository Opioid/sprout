#include "cloth_sample.hpp"
#include "rendering/integrator/surface/integrator_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/lambert/lambert.inl"
#include "sampler/sampler.hpp"
#include "base/math/vector3.inl"
#include "base/math/math.hpp"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace material { namespace cloth {

const material::Sample::Layer& Sample::base_layer() const {
	return layer_;
}

float3 Sample::evaluate(float3_p wi, float& pdf) const {
	float n_dot_wi = layer_.clamped_n_dot(wi);
	pdf = n_dot_wi * math::Pi_inv;
	return pdf * layer_.diffuse_color;
}

float3 Sample::radiance() const {
	return float3::identity();
}

float3 Sample::attenuation() const {
	return float3(100.f, 100.f, 100.f);
}

float Sample::ior() const {
	return 1.5f;
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Result& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	float n_dot_wi = lambert::Isotropic::reflect(layer_.diffuse_color,
												 layer_, sampler, result);
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
