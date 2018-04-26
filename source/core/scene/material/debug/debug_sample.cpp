 #include "debug_sample.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/bxdf.hpp"
#include "sampler/sampler.hpp"
#include "base/math/vector4.inl"
#include "base/math/sampling/sampling.hpp"

namespace scene::material::debug {

constexpr float3 color_front(0.5f, 1.f, 0.f);
constexpr float3 color_back (1.f, 0.f, 0.5f);

const material::Sample::Layer& Sample::base_layer() const {
	return layer_;
}

bxdf::Result Sample::evaluate(f_float3 wi) const {
	const float3 n = math::cross(layer_.t_, layer_.b_);
	const bool same_side = math::dot(n, layer_.n_) > 0.f;

	const float n_dot_wi = layer_.clamp_n_dot(wi);

	const float3 color = same_side ? color_front : color_back;

	const float pdf = n_dot_wi * math::Pi_inv;
	const float3 lambert = math::Pi_inv * color;

	return { n_dot_wi * lambert, pdf };
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Sample& result) const {
	const float3 n = math::cross(layer_.t_, layer_.b_);
	const bool same_side = math::dot(n, layer_.n_) > 0.f;

	const float2 s2d = sampler.generate_sample_2D();

	const float3 is = math::sample_hemisphere_cosine(s2d);

	const float3 wi = math::normalize(layer_.tangent_to_world(is));

	const float n_dot_wi = layer_.clamp_n_dot(wi);

	const float3 color = same_side ? color_front : color_back;

	result.reflection = n_dot_wi * math::Pi_inv * color;
	result.wi = wi;
	result.pdf = n_dot_wi * math::Pi_inv;
	result.type.clear(bxdf::Type::Diffuse_reflection);
}

}
