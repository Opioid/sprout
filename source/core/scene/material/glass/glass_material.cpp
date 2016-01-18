#include "glass_material.hpp"
#include "glass_sample.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace glass {

Glass::Glass(Generic_sample_cache<Sample>& cache, std::shared_ptr<image::texture::Texture_2D> mask) :
	Material(cache, mask, false) {}

const material::Sample& Glass::sample(const shape::Differential& dg, const math::float3& wo, float ior_i,
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

	sample.set(color_, attenuation_distance_, ior_, ior_i);

	return sample;
}

math::float3 Glass::sample_emission(math::float2 /*uv*/, const image::texture::sampler::Sampler_2D& /*sampler*/) const {
	return math::float3::identity;
}

math::float3 Glass::average_emission() const {
	return math::float3::identity;
}

const image::texture::Texture_2D* Glass::emission_map() const {
	return nullptr;
}

float Glass::ior() const {
	return ior_;
}

void Glass::set_normal_map(std::shared_ptr<image::texture::Texture_2D> normal_map) {
	normal_map_ = normal_map;
}

void Glass::set_color(const math::float3& color) {
	color_ = color;
}

void Glass::set_attenuation_distance(float attenuation_distance) {
	attenuation_distance_ = attenuation_distance;
}

void Glass::set_ior(float ior) {
	ior_ = ior;
}

}}}
