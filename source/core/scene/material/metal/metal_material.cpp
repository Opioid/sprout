#include "metal_material.hpp"
#include "metal_sample.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace metal {

Material::Material(Generic_sample_cache<Sample>& cache,
				   std::shared_ptr<image::texture::Texture_2D> mask, bool two_sided) :
	material::Material<Generic_sample_cache<Sample>>(cache, mask, two_sided) {}

const material::Sample& Material::sample(const shape::Differential& dg, const math::float3& wo,
										 const image::texture::sampler::Sampler_2D& sampler,
										 uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

	if (normal_map_) {
		math::float3 nm = sampler.sample_3(*normal_map_, dg.uv);
		math::float3 n = math::normalized(dg.tangent_to_world(nm));

		sample.set_basis(dg.t, dg.b, n, dg.geo_n, wo);
	} else {
		sample.set_basis(dg.t, dg.b, dg.n, dg.geo_n, wo);
	}

	sample.set(color_, roughness_, ior_);

	return sample;
}

math::float3 Material::sample_emission(math::float2 /*uv*/,
									   const image::texture::sampler::Sampler_2D& /*sampler*/) const {
	return math::float3::identity;
}

math::float3 Material::average_emission() const {
	return math::float3::identity;
}

const image::texture::Texture_2D* Material::emission_map() const {
	return nullptr;
}

void Material::set_normal_map(std::shared_ptr<image::texture::Texture_2D> normal_map) {
	normal_map_ = normal_map;
}

void Material::set_color(const math::float3& color) {
	color_ = color;
}

void Material::set_roughness(float roughness) {
	roughness_ = roughness;
}

void Material::set_ior(float ior) {
	ior_ = ior;
}

}}}

