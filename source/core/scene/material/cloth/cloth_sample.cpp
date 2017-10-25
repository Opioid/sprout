#include "cloth_sample.hpp"
#include "rendering/integrator/surface/integrator_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/lambert/lambert.inl"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector4.inl"
#include "base/math/sampling/sampling.hpp"

namespace scene::material::cloth {

const material::Sample::Layer& Sample::base_layer() const {
	return layer_;
}

bxdf::Result Sample::evaluate(const float3& wi) const {
	const float n_dot_wi = layer_.clamp_n_dot(wi);
	const float pdf = n_dot_wi * math::Pi_inv;
	return { pdf * layer_.diffuse_color, pdf };
}

float Sample::ior() const {
	return 1.5f;
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Sample& result) const {
	if (!same_hemisphere(wo_)) {
		result.pdf = 0.f;
		return;
	}

	const float n_dot_wi = lambert::Isotropic::reflect(layer_.diffuse_color,
													   layer_, sampler, result);
	result.reflection *= n_dot_wi;
}

void Sample::Layer::set(const float3& color) {
	this->diffuse_color = color;
}

}
