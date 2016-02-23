#include "cloth_material.hpp"
#include "cloth_sample.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace cloth {

Material::Material(Generic_sample_cache<Sample>& cache,
				   std::shared_ptr<image::texture::Texture_2D> mask, bool two_sided) :
	material::Material<Generic_sample_cache<Sample>>(cache, mask, two_sided) {}

const material::Sample& Material::sample(const shape::Differential& dg, const math::vec3& wo,
										 float /*time*/, float /*ior_i*/,
										 const image::texture::sampler::Sampler_2D& sampler,
										 uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	if (normal_map_) {
		math::vec3 nm = sampler.sample_3(*normal_map_, dg.uv);
		math::vec3 n = math::normalized(dg.tangent_to_world(nm));

		sample.set_basis(dg.t, dg.b, n, dg.geo_n, wo, two_sided_);
	} else {
		sample.set_basis(dg.t, dg.b, dg.n, dg.geo_n, wo, two_sided_);
	}

	math::vec3 color;

	if (color_map_) {
		color = sampler.sample_3(*color_map_, dg.uv);
	} else {
		color = color_;
	}

	sample.set(color);

	return sample;
}

math::vec3 Material::sample_emission(math::float2 /*uv*/, float /*time*/,
									   const image::texture::sampler::Sampler_2D& /*sampler*/) const {
	return math::vec3_identity;
}

math::vec3 Material::average_emission() const {
	return math::vec3_identity;
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

void Material::set_color(const math::vec3& color) {
	color_ = color;
}

}}}
