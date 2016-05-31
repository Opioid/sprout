#include "sky_material.hpp"
#include "sky_model.hpp"
#include "core/scene/scene_worker.hpp"
#include "core/scene/material/material_sample.inl"
#include "core/scene/material/material_sample_cache.inl"
#include "core/scene/shape/geometry/hitpoint.inl"
#include "base/math/vector.inl"

namespace procedural { namespace sky {

Material::Material(scene::material::Generic_sample_cache<scene::material::light::Sample>& cache,
				   Model& model) :
	scene::material::Typed_material<
		scene::material::Generic_sample_cache<scene::material::light::Sample>>(
			cache, nullptr, scene::material::Sampler_settings(), false),
	model_(model) {}

const scene::material::Sample& Material::sample(const scene::shape::Hitpoint& hp,
												math::pfloat3 wo, float /*area*/, float /*time*/,
												float /*ior_i*/, const scene::Worker& worker,
												Sampler_filter /*filter*/) {
	auto& sample = cache_.get(worker.id());

	sample.set_basis(hp.t, hp.b, hp.n, hp.geo_n, wo);

	sample.set(model_.evaluate(-wo));

	return sample;
}

math::float3 Material::sample_radiance(math::pfloat3 wi, math::float2 /*uv*/,
									   float /*area*/, float /*time*/,
									   const scene::Worker& /*worker*/,
									   Sampler_filter /*filter*/) const {
	return model_.evaluate(wi);
}

math::float3 Material::average_radiance(float /*area*/) const {
	return math::float3(1.f, 1.f, 1.f);
}

bool Material::has_emission_map() const {
	return false;
}

void Material::prepare_sampling(bool /*spherical*/) {
	model_.init();
}

}}
