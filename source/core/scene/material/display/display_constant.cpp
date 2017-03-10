#include "display_constant.hpp"
#include "display_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/shape/shape.hpp"
#include "base/math/math.hpp"
#include "base/math/vector4.inl"
#include "base/math/distribution/distribution_2d.inl"
#include "base/spectrum/rgb.hpp"

namespace scene { namespace material { namespace display {

Constant::Constant(Sample_cache& sample_cache, const Sampler_settings& sampler_settings,
				   bool two_sided) :
	material::Material(sample_cache, sampler_settings, two_sided) {}

const material::Sample& Constant::sample(float3_p wo, const Renderstate& rs,
										 const Worker& worker, Sampler_filter /*filter*/) {
	auto& sample = sample_cache_.get<Sample>(worker.id());

	sample.set_basis(rs.geo_n, wo);

	sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

	sample.layer_.set(emission_, f0_, roughness_);

	return sample;
}

float3 Constant::sample_radiance(float3_p /*wi*/, float2 /*uv*/, float /*area*/,
								 float /*time*/, const Worker& /*worker*/,
								 Sampler_filter /*filter*/) const {
	return emission_;
}

float3 Constant::average_radiance(float /*area*/) const {
	return emission_;
}

size_t Constant::num_bytes() const {
	return sizeof(*this);
}

void Constant::set_emission(float3_p radiance) {
	emission_ = radiance;
}

void Constant::set_roughness(float roughness) {
	roughness_ = roughness;
}

void Constant::set_ior(float ior) {
	f0_ = fresnel::schlick_f0(1.f, ior);
}

}}}
