#include "display_emissionmap.hpp"
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

namespace scene { namespace material { namespace display {

Emissionmap::Emissionmap(const Sampler_settings& sampler_settings, bool two_sided) :
	light::Emissionmap(sampler_settings, two_sided) {}

const material::Sample& Emissionmap::sample(const float3& wo, const Renderstate& rs,
											Sampler_filter filter, Worker& worker) {
	auto& sample = worker.sample<Sample>();

	sample.set_basis(rs.geo_n, wo);

	sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

	auto& sampler = worker.sampler_2D(sampler_key(), filter);

	float3 radiance = emission_map_.sample_3(sampler, rs.uv);
	sample.layer_.set(emission_factor_ * radiance, f0_, roughness_);

	return sample;
}

size_t Emissionmap::num_bytes() const {
	return sizeof(*this);
}

void Emissionmap::set_roughness(float roughness) {
	roughness_ = roughness;
}

void Emissionmap::set_ior(float ior) {
	f0_ = fresnel::schlick_f0(1.f, ior);
}

}}}
