#include "matte_material.hpp"
#include "matte_sample.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "base/math/vector3.inl"

namespace scene { namespace material { namespace matte {

Material::Material(Sample_cache& sample_cache, const Sampler_settings& sampler_settings,
				   bool two_sided) :
	material::Material(sample_cache, sampler_settings, two_sided) {}

const material::Sample& Material::sample(float3_p wo, const Renderstate& rs,
										 const Worker& worker, Sampler_filter /*filter*/) {
	auto& sample = sample_cache_.get<Sample>(worker.id());

//	auto& sampler = worker.sampler_2D(sampler_key_, filter);

	sample.set_basis(rs.geo_n, wo);

	sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

	sample.layer_.set(color_);

	return sample;
}

size_t Material::num_bytes() const {
	return sizeof(*this);
}

void Material::set_color(float3_p color) {
	color_ = color;
}

}}}
