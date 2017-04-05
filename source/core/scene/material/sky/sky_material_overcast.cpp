#include "sky_material_overcast.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "base/math/vector3.inl"

namespace scene { namespace material { namespace sky {

Material_overcast::Material_overcast(Sample_cache& sample_cache,
									 const Sampler_settings& sampler_settings,
									 bool two_sided) :
	Material(sample_cache, sampler_settings, two_sided) {}

const material::Sample& Material_overcast::sample(const float3& wo, const Renderstate& rs,
												  const Worker& worker, Sampler_filter /*filter*/) {
	auto& sample = sample_cache_.get<light::Sample>(worker.id());

	sample.set_basis(rs.geo_n, wo);
	sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

	sample.layer_.set(overcast(-wo));

	return sample;
}

float3 Material_overcast::sample_radiance(const float3& wi, float2 /*uv*/,
										  float /*area*/, float /*time*/,
										  const Worker& /*worker*/,
										  Sampler_filter /*filter*/) const {
	return overcast(wi);
}

float3 Material_overcast::average_radiance(float /*area*/) const {
	if (is_two_sided()) {
		return 2.f * color_;
	}

	return color_;
}

size_t Material_overcast::num_bytes() const {
	return sizeof(*this);
}

void Material_overcast::set_emission(const float3& radiance) {
	color_ = radiance;
}

float3 Material_overcast::overcast(const float3& wi) const {
	return ((1.f + 2.f * math::dot(float3(0.f, 1.f, 0.f), wi)) / 3.f) * color_;
}

}}}
