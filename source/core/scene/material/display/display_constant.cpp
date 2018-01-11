#include "display_constant.hpp"
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

Constant::Constant(const Sampler_settings& sampler_settings, bool two_sided) :
	material::Material(sampler_settings, two_sided) {}

const material::Sample& Constant::sample(const float3& wo, const Renderstate& rs,
										 Sampler_filter /*filter*/, sampler::Sampler& /*sampler*/,
										 const Worker& worker) const {
	auto& sample = worker.sample<Sample>();

	sample.set_basis(rs.geo_n, wo);

	sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

	sample.layer_.set(emission_, f0_, roughness_);

	return sample;
}

float3 Constant::sample_radiance(const float3& /*wi*/, float2 /*uv*/, float /*area*/,
								 float /*time*/, Sampler_filter /*filter*/,
								 const Worker& /*worker*/) const {
	return emission_;
}

float3 Constant::average_radiance(float /*area*/) const {
	return emission_;
}

size_t Constant::num_bytes() const {
	return sizeof(*this);
}

void Constant::set_emission(const float3& radiance) {
	emission_ = radiance;
}

void Constant::set_roughness(float roughness) {
	roughness_ = roughness;
}

void Constant::set_ior(float ior) {
	f0_ = fresnel::schlick_f0(1.f, ior);
}

}
