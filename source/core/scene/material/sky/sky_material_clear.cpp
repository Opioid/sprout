#include "sky_material_clear.hpp"
#include "sky_sample_clear.hpp"
#include "scene/scene_worker.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/hitpoint.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace sky {

Material_clear::Material_clear(Generic_sample_cache<Sample_clear>& cache,
							   std::shared_ptr<image::texture::Texture_2D> mask,
							   const Sampler_settings& sampler_settings, bool two_sided) :
	material::Typed_material<Generic_sample_cache<Sample_clear>>(cache, mask, sampler_settings, two_sided) {}

const material::Sample& Material_clear::sample(const shape::Hitpoint& hp, math::pfloat3 wo,
											   float /*time*/, float /*ior_i*/,
											   const Worker& worker, Sampler_settings::Filter /*filter*/) {
	auto& sample = cache_.get(worker.id());

	sample.set_basis(hp.t, hp.b, hp.n, hp.geo_n, wo);
	sample.set(math::float3(0.f, 1.f, 0.f), emission_);

	return sample;
}

math::float3 Material_clear::sample_emission(math::float2 /*uv*/, float /*time*/,
											 const Worker& /*worker*/, Sampler_settings::Filter /*filter*/) const {
	return emission_;
}

math::float3 Material_clear::average_emission() const {
	if (is_two_sided()) {
		return 2.f * emission_;
	}

	return emission_;
}

bool Material_clear::has_emission_map() const {
	return false;
}

void Material_clear::set_emission(math::pfloat3 emission) {
	emission_ = emission;
}

}}}
