#include "display_emissionmap_animated.hpp"
#include "display_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "scene/material/material_sample.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/shape/shape.hpp"
#include "base/math/math.hpp"
#include "base/math/vector4.inl"
#include "base/math/distribution/distribution_2d.inl"
#include "base/spectrum/rgb.hpp"

namespace scene::material::display {

Emissionmap_animated::Emissionmap_animated(const Sampler_settings& sampler_settings,
										   bool two_sided, const Texture_adapter& emission_map,
										   float emission_factor, float animation_duration) :
	light::Emissionmap_animated(sampler_settings, two_sided,
								emission_map, emission_factor, animation_duration)
{}

const material::Sample& Emissionmap_animated::sample(const float3& wo, const Renderstate& rs,
													 Sampler_filter filter,
													 sampler::Sampler& /*sampler*/,
													 const Worker& worker) const {
	auto& sample = worker.sample<Sample>();

	sample.set_basis(rs.geo_n, wo);

	sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

	auto& sampler = worker.sampler_2D(sampler_key(), filter);

	float3 radiance = emission_map_.sample_3(sampler, rs.uv, element_);
	sample.layer_.set(emission_factor_ * radiance, f0_, roughness_);

	return sample;
}

float Emissionmap_animated::ior() const {
	return ior_;
}

size_t Emissionmap_animated::num_bytes() const {
	return sizeof(*this);
}

void Emissionmap_animated::set_roughness(float roughness) {
	roughness_ = roughness;
}

void Emissionmap_animated::set_ior(float ior) {
	ior_ = ior;
	f0_ = fresnel::schlick_f0(1.f, ior);
}

}
