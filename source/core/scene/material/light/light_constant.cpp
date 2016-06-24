#include "light_constant.hpp"
#include "light_material_sample.hpp"
#include "scene/scene_worker.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/hitpoint.inl"
#include "base/spectrum/rgb.inl"

namespace scene { namespace material { namespace light {

Constant::Constant(Generic_sample_cache<Sample>& cache, Texture_2D_ptr mask,
				   const Sampler_settings& sampler_settings, bool two_sided) :
	Material(cache, mask, sampler_settings, two_sided) {}

const material::Sample& Constant::sample(const shape::Hitpoint& hp, float3_p wo,
										 float area, float /*time*/, float /*ior_i*/,
										 const Worker& worker, Sampler_filter /*filter*/) {
	auto& sample = cache_.get(worker.id());

	float side = sample.set_basis(hp.geo_n, wo, two_sided_);

	sample.layer_.set_basis(hp.t, hp.b, hp.n, side);

	sample.layer_.set(emittance_.radiance(area));

	return sample;
}

float3 Constant::sample_radiance(float3_p /*wi*/, float2 /*uv*/, float area, float /*time*/,
								 const Worker& /*worker*/, Sampler_filter /*filter*/) const {
	return emittance_.radiance(area);
}

float3 Constant::average_radiance(float area) const {
	float3 radiance = emittance_.radiance(area);

	if (is_two_sided()) {
		return 2.f * radiance;
	}

	return radiance;
}

bool Constant::has_emission_map() const {
	return false;
}

::light::Emittance& Constant::emittance() {
	return emittance_;
}

}}}
