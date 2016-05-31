#include "sky_material_overcast.hpp"
#include "scene/scene_worker.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/hitpoint.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace sky {

Material_overcast::Material_overcast(Generic_sample_cache<light::Sample>& cache,
									 std::shared_ptr<image::texture::Texture_2D> mask,
									 const Sampler_settings& sampler_settings, bool two_sided) :
	material::Typed_material<Generic_sample_cache<light::Sample>>(
		cache, mask, sampler_settings, two_sided) {}

const material::Sample& Material_overcast::sample(const shape::Hitpoint& hp, math::pfloat3 wo,
												  float /*area*/, float /*time*/, float /*ior_i*/,
												  const Worker& worker, Sampler_filter /*filter*/) {
	auto& sample = cache_.get(worker.id());

	sample.set_basis(hp.t, hp.b, hp.n, hp.geo_n, wo);

	sample.set(overcast(-wo));

	return sample;
}

math::float3 Material_overcast::sample_radiance(math::pfloat3 wi, math::float2 /*uv*/,
												float /*area*/, float /*time*/,
												const Worker& /*worker*/,
												Sampler_filter /*filter*/) const {
	return overcast(wi);
}

math::float3 Material_overcast::average_radiance(float /*area*/) const {
	if (is_two_sided()) {
		return 2.f * color_;
	}

	return color_;
}

void Material_overcast::set_emission(math::pfloat3 radiance) {
	color_ = radiance;
}

math::float3 Material_overcast::overcast(math::pfloat3 wi) const {
	return ((1.f + 2.f * math::dot(math::float3(0.f, 1.f, 0.f), wi)) / 3.f) * color_;
}

}}}
