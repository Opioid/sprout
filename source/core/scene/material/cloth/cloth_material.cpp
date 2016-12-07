#include "cloth_material.hpp"
#include "cloth_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_renderstate.inl"
#include "scene/scene_worker.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace cloth {

Material::Material(Sample_cache<Sample>& cache,
				   const Sampler_settings& sampler_settings, bool two_sided) :
	material::Typed_material<Sample_cache<Sample>>(cache,
														   sampler_settings,
														   two_sided) {}

const material::Sample& Material::sample(float3_p wo, const Renderstate& rs,
										 const Worker& worker, Sampler_filter filter) {
	auto& sample = cache_.get(worker.id());

	auto& sampler = worker.sampler_2D(sampler_key(), filter);

	sample.set_basis(rs.geo_n, wo);

	if (normal_map_.is_valid()) {
		float3 nm = normal_map_.sample_3(sampler, rs.uv);
		float3 n = math::normalized(rs.tangent_to_world(nm));
		sample.layer_.set_tangent_frame(rs.t, rs.b, n);
	} else {
		sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
	}

	float3 color;
	if (color_map_.is_valid()) {
		color = color_map_.sample_3(sampler, rs.uv);
	} else {
		color = color_;
	}

	sample.layer_.set(color);

	return sample;
}

size_t Material::num_bytes() const {
	return sizeof(*this);
}

void Material::set_color_map(const Texture_adapter& color_map) {
	color_map_ = color_map;
}

void Material::set_normal_map(const Texture_adapter& normal_map) {
	normal_map_ = normal_map;
}

void Material::set_color(float3_p color) {
	color_ = color;
}

}}}
