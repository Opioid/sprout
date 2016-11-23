#include "sky_material_overcast.hpp"
#include "scene/scene_renderstate.inl"
#include "scene/scene_worker.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace sky {

Material_overcast::Material_overcast(Sample_cache<light::Sample>& cache,
									 const Sampler_settings& sampler_settings, bool two_sided) :
	material::Typed_material<Sample_cache<light::Sample>>(
		cache, sampler_settings, two_sided) {}

const material::Sample& Material_overcast::sample(float3_p wo, const Renderstate& rs,
												  const Worker& worker, Sampler_filter /*filter*/) {
	auto& sample = cache_.get(worker.id());

	sample.set_basis(rs.geo_n, wo);
	sample.layer_.set_basis(rs.t, rs.b, rs.n);

	sample.layer_.set(overcast(-wo));

	return sample;
}

float3 Material_overcast::sample_radiance(float3_p wi, float2 /*uv*/,
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

void Material_overcast::set_emission(float3_p radiance) {
	color_ = radiance;
}

float3 Material_overcast::overcast(float3_p wi) const {
	return ((1.f + 2.f * math::dot(float3(0.f, 1.f, 0.f), wi)) / 3.f) * color_;
}

}}}
