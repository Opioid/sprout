#include "substitute_material.hpp"
#include "substitute_sample.hpp"
#include "substitute_base_material.inl"
#include "substitute_base_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.hpp"
#include "base/math/vector4.inl"

namespace scene { namespace material { namespace substitute {

Material::Material(Sample_cache& sample_cache, const Sampler_settings& sampler_settings,
				   bool two_sided) :
	Material_base(sample_cache, sampler_settings, two_sided) {}

const material::Sample& Material::sample(float3_p wo, const Renderstate& rs,
										 const Worker& worker, Sampler_filter filter) {
	auto& sample = sample_cache_.get<Sample>(worker.id());

	auto& sampler = worker.sampler_2D(sampler_key(), filter);

	set_sample(wo, rs, sampler, sample);

	return sample;
}

size_t Material::num_bytes() const {
	return sizeof(*this);
}

}}}
