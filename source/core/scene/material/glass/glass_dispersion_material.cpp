#include "glass_dispersion_material.hpp"
#include "glass_dispersion_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "scene/material/material_helper.hpp"
#include "scene/material/material_sample.inl"
#include "base/math/vector4.inl"

namespace scene::material::glass {

Glass_dispersion::Glass_dispersion(const Sampler_settings& sampler_settings) :
	Glass(sampler_settings) {}

const material::Sample& Glass_dispersion::sample(f_float3 wo, const Renderstate& rs,
												 Sampler_filter filter,
												 sampler::Sampler& /*sampler*/,
												 const Worker& worker) const {
	auto& sample = worker.sample<Sample_dispersion>();

	sample.set_basis(rs.geo_n, wo);

	if (normal_map_.is_valid()) {
		auto& sampler = worker.sampler_2D(sampler_key(), filter);
		float3 const n = sample_normal(wo, rs, normal_map_, sampler);
		sample.layer_.set_tangent_frame(n);
	} else {
		sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
	}

	sample.set(ior_, abbe_, rs.wavelength);
	sample.layer_.set(refraction_color_, absorption_color_, attenuation_distance_, ior_, rs.ior);

	return sample;
}

size_t Glass_dispersion::num_bytes() const {
	return sizeof(*this);
}

void Glass_dispersion::set_abbe(float abbe) {
	abbe_ = abbe;
}

size_t Glass_dispersion::sample_size() {
	return sizeof(Sample_dispersion);
}

}
