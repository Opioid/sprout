#include "debug_material.hpp"
#include "debug_sample.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "image/texture/sampler/sampler_2d.hpp"
#include "scene/material/material_sample.inl"
#include "base/math/vector3.inl"

namespace scene { namespace material { namespace debug {

Material::Material(const Sampler_settings& sampler_settings) :
	material::Material(sampler_settings, true) {}

const material::Sample& Material::sample(const float3& wo, const Renderstate& rs,
										 Worker& worker, Sampler_filter /*filter*/) {
	auto& sample = worker.sample<Sample>();

	sample.set_basis(rs.geo_n, wo);

	sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

	return sample;
}

size_t Material::num_bytes() const {
	return sizeof(*this);
}

}}}
