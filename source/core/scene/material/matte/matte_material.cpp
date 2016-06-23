#include "matte_material.hpp"
#include "matte_sample.hpp"
#include "scene/scene_worker.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/hitpoint.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace matte {

Material::Material(Generic_sample_cache<Sample>& cache,
				   Texture_2D_ptr mask,
				   const Sampler_settings& sampler_settings, bool two_sided) :
	material::Typed_material<Generic_sample_cache<Sample>>(cache, mask,
														   sampler_settings, two_sided) {}

const material::Sample& Material::sample(const shape::Hitpoint& hp, float3_p wo,
										 float /*area*/, float /*time*/, float /*ior_i*/,
										 const Worker& worker, Sampler_filter /*filter*/) {
	auto& sample = cache_.get(worker.id());

//	auto& sampler = worker.sampler(sampler_key_, filter);

	sample.set_basis(hp.t, hp.b, hp.n, hp.geo_n, wo);

	sample.set(color_);

	return sample;
}

void Material::set_color(float3_p color) {
	color_ = color;
}

}}}
