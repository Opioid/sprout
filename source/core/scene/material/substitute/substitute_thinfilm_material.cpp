#include "substitute_thinfilm_material.hpp"
#include "substitute_thinfilm_sample.hpp"
#include "substitute_base_material.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/material/coating/coating.inl"

namespace scene { namespace material { namespace substitute {

Material_thinfilm::Material_thinfilm(Generic_sample_cache<Sample_thinfilm>& cache,
									 Texture_2D_ptr mask,
									 const Sampler_settings& sampler_settings, bool two_sided) :
	Material_base<Sample_thinfilm>(cache, mask, sampler_settings, two_sided) {}

const material::Sample& Material_thinfilm::sample(float3_p wo, const Renderstate& rs,
												  const Worker& worker, Sampler_filter filter) {
	auto& sample = cache_.get(worker.id());

	auto& sampler = worker.sampler(sampler_key_, filter);

	set_sample(wo, rs, sampler, sample);

	sample.coating_.set(thinfilm_.ior, thinfilm_.a2, thinfilm_.thickness, thinfilm_.weight);

	return sample;
}

void Material_thinfilm::set_thinfilm(float ior, float roughness, float thickness, float weight) {
	thinfilm_.ior = ior;
	thinfilm_.a2  = math::pow4(roughness);
	thinfilm_.thickness = thickness;
	thinfilm_.weight = weight;
}

}}}
