#include "substitute_material.hpp"
#include "substitute_sample.hpp"
#include "substitute_base_material.inl"
#include "substitute_base_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "base/math/vector4.inl"

#include "base/debug/assert.hpp"

namespace scene::material::substitute {

Material::Material(const Sampler_settings& sampler_settings, bool two_sided) :
	Material_base(sampler_settings, two_sided) {}

const material::Sample& Material::sample(f_float3 wo, const Renderstate& rs,
										 Sampler_filter filter, sampler::Sampler& /*sampler*/,
										 const Worker& worker) const {
	SOFT_ASSERT(!rs.subsurface);

	auto& sample = worker.sample<Sample>();

	auto& sampler = worker.sampler_2D(sampler_key(), filter);

	set_sample(wo, rs, sampler, sample);

	return sample;
}

size_t Material::num_bytes() const {
	return sizeof(*this);
}

size_t Material::sample_size() {
	return sizeof(Sample);
}

}
