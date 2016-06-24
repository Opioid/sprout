#include "substitute_clearcoat_material.hpp"
#include "substitute_clearcoat_sample.hpp"
#include "substitute_base_material.inl"
#include "scene/scene_renderstate.hpp"

namespace scene { namespace material { namespace substitute {

Material_clearcoat::Material_clearcoat(Generic_sample_cache<Sample_clearcoat>& cache,
									   Texture_2D_ptr mask,
									   const Sampler_settings& sampler_settings, bool two_sided) :
	Material_base<Sample_clearcoat>(cache, mask, sampler_settings, two_sided) {}

const material::Sample& Material_clearcoat::sample(float3_p wo, const Renderstate& rs,
												   const Worker& worker, Sampler_filter filter) {
	auto& sample = cache_.get(worker.id());

	auto& sampler = worker.sampler(sampler_key_, filter);

	set_sample(rs, wo, sampler, sample);

	sample.set_clearcoat(clearcoat_);

	return sample;
}

void Material_clearcoat::set_clearcoat(float ior, float roughness, float weight) {
	clearcoat_.f0 = fresnel::schlick_f0(1.f, ior);
	clearcoat_.a2 = math::pow4(roughness);
	clearcoat_.weight = weight;
}

}}}
