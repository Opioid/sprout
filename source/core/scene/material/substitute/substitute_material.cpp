#include "substitute_material.hpp"
#include "substitute_sample.hpp"
#include "substitute_base_material.inl"
#include "scene/scene_renderstate.hpp"

namespace scene { namespace material { namespace substitute {

Material::Material(Generic_sample_cache<Sample>& cache, Texture_2D_ptr mask,
				   const Sampler_settings& sampler_settings, bool two_sided) :
	Material_base<Sample>(cache, mask, sampler_settings, two_sided) {}

const material::Sample& Material::sample(float3_p wo, const Renderstate& rs,
										 const Worker& worker, Sampler_filter filter) {
	auto& sample = cache_.get(worker.id());

	auto& sampler = worker.sampler(sampler_key_, filter);

	set_sample(rs, wo, sampler, sample);

	return sample;
}

}}}
