#include "sky_material_clear.hpp"
#include "scene/scene_worker.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/hitpoint.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace sky {

Material_clear::Material_clear(Generic_sample_cache<light::Sample>& cache,
							   std::shared_ptr<image::texture::Texture_2D> mask,
							   const Sampler_settings& sampler_settings, bool two_sided) :
	material::Typed_material<Generic_sample_cache<light::Sample>>(
		cache, mask, sampler_settings, two_sided) {}

const material::Sample& Material_clear::sample(const shape::Hitpoint& hp, math::pfloat3 wo,
											   float /*time*/, float /*ior_i*/,
											   const Worker& worker,
											   Sampler_settings::Filter /*filter*/) {
	auto& sample = cache_.get(worker.id());

	sample.set_basis(hp.t, hp.b, hp.n, hp.geo_n, wo);

	math::float3 radiance = model_.evaluate(-wo);

	sample.set(radiance);

	return sample;
}

math::float3 Material_clear::sample_emission(math::pfloat3 wi, math::float2 /*uv*/,
											 float /*time*/, const Worker& /*worker*/,
											 Sampler_settings::Filter /*filter*/) const {
	return model_.evaluate(wi);
}

math::float3 Material_clear::average_emission() const {
	return math::float3(1.f, 1.f, 1.f);
}

bool Material_clear::has_emission_map() const {
	return false;
}

void Material_clear::prepare_sampling(bool /*spherical*/) {
	model_.init();
}

void Material_clear::set_sun_direction(math::pfloat3 direction) {
	model_.set_sun_direction(direction);
}

void Material_clear::set_ground_albedo(math::pfloat3 albedo) {
	model_.set_ground_albedo(albedo);
}

void Material_clear::set_turbidity(float turbidity) {
	model_.set_turbidity(turbidity);
}

}}}
