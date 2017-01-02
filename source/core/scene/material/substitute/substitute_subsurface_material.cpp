#include "substitute_subsurface_material.hpp"
#include "substitute_subsurface_sample.hpp"
#include "substitute_base_material.inl"
#include "scene/scene_renderstate.hpp"

namespace scene { namespace material { namespace substitute {

Material_subsurface::Material_subsurface(Sample_cache<Sample_subsurface>& cache,
										 const Sampler_settings& sampler_settings, bool two_sided) :
	Material_base<Sample_subsurface>(cache, sampler_settings, two_sided) {}

const material::Sample& Material_subsurface::sample(float3_p wo, const Renderstate& rs,
													const Worker& worker, Sampler_filter filter) {
	auto& sample = cache_.get(worker.id());

	auto& sampler = worker.sampler_2D(sampler_key(), filter);

	set_sample(wo, rs, sampler, sample);

	return sample;
}

bool Material_subsurface::is_subsurface() const {
	return true;
}

size_t Material_subsurface::num_bytes() const {
	return sizeof(*this);
}

void Material_subsurface::set_scattering(float3_p scattering) {
	scattering_ = scattering;
}

}}}

