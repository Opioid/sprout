#include "cloth_material.hpp"
#include "cloth_sample.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "scene/scene_worker.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace cloth {

Material::Material(Generic_sample_cache<Sample>& cache, std::shared_ptr<image::texture::Texture_2D> mask,
				   const Sampler_settings& sampler_settings, bool two_sided) :
	material::Typed_material<Generic_sample_cache<Sample>>(cache, mask, sampler_settings, two_sided) {}

const material::Sample& Material::sample(const shape::Differential& dg, const math::float3& wo,
										 float /*time*/, float /*ior_i*/,
										 const Worker& worker, Sampler_settings::Filter filter) {
	auto& sample = cache_.get(worker.id());

	auto& sampler = worker.sampler(sampler_key_, filter);

	if (normal_map_) {
		math::float3 nm = sampler.sample_3(*normal_map_, dg.uv);
		math::float3 n = math::normalized(dg.tangent_to_world(nm));

		sample.set_basis(dg.t, dg.b, n, dg.geo_n, wo, two_sided_);
	} else {
		sample.set_basis(dg.t, dg.b, dg.n, dg.geo_n, wo, two_sided_);
	}

	math::float3 color;

	if (color_map_) {
		color = sampler.sample_3(*color_map_, dg.uv);
	} else {
		color = color_;
	}

	sample.set(color);

	return sample;
}

math::float3 Material::sample_emission(math::float2 /*uv*/, float /*time*/,
									   const Worker& /*worker*/, Sampler_settings::Filter /*filter*/) const {
	return math::float3_identity;
}

math::float3 Material::average_emission() const {
	return math::float3_identity;
}

bool Material::has_emission_map() const {
	return false;
}

void Material::set_color_map(std::shared_ptr<image::texture::Texture_2D> color_map) {
	color_map_ = color_map;
}

void Material::set_normal_map(std::shared_ptr<image::texture::Texture_2D> normal_map) {
	normal_map_ = normal_map;
}

void Material::set_color(const math::float3& color) {
	color_ = color;
}

}}}
