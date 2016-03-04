#include "light_constant.hpp"
#include "light_material_sample.hpp"
#include "scene/scene_worker.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"
#include "base/color/color.inl"

namespace scene { namespace material { namespace light {

Constant::Constant(Generic_sample_cache<Sample>& cache,
				   std::shared_ptr<image::texture::Texture_2D> mask,
				   const Sampler_settings& sampler_settings, bool two_sided,
				   const math::float3& emission) :
	Material(cache, mask, sampler_settings, two_sided), emission_(emission) {}

const material::Sample& Constant::sample(const shape::Differential& dg, const math::float3& wo,
										 float /*time*/, float /*ior_i*/,
										 const Worker& worker, Sampler_settings::Filter /*filter*/) {
	auto& sample = cache_.get(worker.id());

	sample.set_basis(dg.t, dg.b, dg.n, dg.geo_n, wo, two_sided_);
	sample.set(emission_);

	return sample;
}

math::float3 Constant::sample_emission(math::float2 /*uv*/, float /*time*/,
									   const Worker& /*worker*/, Sampler_settings::Filter /*filter*/) const {
	return emission_;
}

math::float3 Constant::average_emission() const {
	if (is_two_sided()) {
		return 2.f * emission_;
	}

	return emission_;
}

bool Constant::has_emission_map() const {
	return false;
}

}}}
