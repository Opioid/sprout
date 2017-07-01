#include "substitute_subsurface_material.hpp"
#include "substitute_subsurface_sample.hpp"
#include "substitute_base_sample.inl"
#include "substitute_base_material.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.hpp"
#include "scene/material/bssrdf.hpp"
#include "base/math/vector4.inl"

namespace scene { namespace material { namespace substitute {

Material_subsurface::Material_subsurface(const Sampler_settings& sampler_settings) :
	Material_base(sampler_settings, false) {}

const material::Sample& Material_subsurface::sample(const float3& wo, const Renderstate& rs,
													Worker& worker, Sampler_filter filter) {
	auto& sample = worker.sample_cache().get<Sample_subsurface>();

	auto& sampler = worker.sampler_2D(sampler_key(), filter);

	set_sample(wo, rs, sampler, sample);

	sample.set(ior_, rs.ior);

	return sample;
}

const BSSRDF& Material_subsurface::bssrdf(Worker& worker) {
	auto& bssrdf = worker.sample_cache().bssrdf();

	bssrdf.set(absorption_, scattering_);

	return bssrdf;
}

bool Material_subsurface::is_subsurface() const {
	return true;
}

size_t Material_subsurface::num_bytes() const {
	return sizeof(*this);
}

void Material_subsurface::set_absorption(const float3& absorption) {
//	absorption_ = absorption;
	absorption_ = material::absorption_coefficient(float3(1.f, 1.f, 1.f), 0.1f);
}

void Material_subsurface::set_scattering(const float3& scattering) {
//	scattering_ = scattering;
	scattering_ = material::scattering_coefficient(float3(1.f, 1.f, 1.f), 0.1f);
}

}}}

