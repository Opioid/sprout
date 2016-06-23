#include "glass_material.hpp"
#include "glass_sample.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "scene/scene_worker.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/hitpoint.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace glass {

Glass::Glass(Generic_sample_cache<Sample>& cache, std::shared_ptr<image::texture::Texture_2D> mask,
			 const Sampler_settings& sampler_settings) :
	Typed_material(cache, mask, sampler_settings, false) {}

const material::Sample& Glass::sample(const shape::Hitpoint& hp, float3_p wo,
									  float /*area*/, float /*time*/, float ior_i,
									  const Worker& worker, Sampler_filter filter) {
	auto& sample = cache_.get(worker.id());

	if (normal_map_) {
		auto& sampler = worker.sampler(sampler_key_, filter);

		float3 nm = sampler.sample_3(*normal_map_, hp.uv);
		float3 n = math::normalized(hp.tangent_to_world(nm));

		sample.set_basis(hp.t, hp.b, n, hp.geo_n, wo);
	} else {
		sample.set_basis(hp.t, hp.b, hp.n, hp.geo_n, wo);
	}

	sample.set(color_, attenuation_distance_, ior_, ior_i);

	return sample;
}

void Glass::set_normal_map(std::shared_ptr<image::texture::Texture_2D> normal_map) {
	normal_map_ = normal_map;
}

void Glass::set_color(const float3& color) {
	color_ = color;
}

void Glass::set_attenuation_distance(float attenuation_distance) {
	attenuation_distance_ = attenuation_distance;
}

void Glass::set_ior(float ior) {
	ior_ = ior;
}

}}}
