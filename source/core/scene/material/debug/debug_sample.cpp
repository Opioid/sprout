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
	float3 const n = math::cross(layer_.t_, layer_.b_);
	bool const same_side = math::dot(n, layer_.n_) > 0.f;

	float const n_dot_wi = layer_.clamp_n_dot(wi);

	float3 const color = same_side ? color_front : color_back;

	float const pdf = n_dot_wi * math::Pi_inv;
	float3 const lambert = math::Pi_inv * color;

	return { n_dot_wi * lambert, pdf };
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Sample& result) const {
	float3 const n = math::cross(layer_.t_, layer_.b_);
	bool const same_side = math::dot(n, layer_.n_) > 0.f;

	float2 const s2d = sampler.generate_sample_2D();

	float3 const is = math::sample_hemisphere_cosine(s2d);

	float3 const wi = math::normalize(layer_.tangent_to_world(is));

	float const n_dot_wi = layer_.clamp_n_dot(wi);

	float3 const color = same_side ? color_front : color_back;

	result.reflection = n_dot_wi * math::Pi_inv * color;
	result.wi = wi;
	result.pdf = n_dot_wi * math::Pi_inv;
	result.type.clear(bxdf::Type::Diffuse_reflection);
}

}
