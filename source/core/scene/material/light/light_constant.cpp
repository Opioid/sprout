#include "light_constant.hpp"
#include "light_material_sample.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "scene/material/material_sample.inl"
#include "base/math/vector4.inl"
#include "base/spectrum/rgb.hpp"

namespace scene { namespace material { namespace light {

Constant::Constant(const Sampler_settings& sampler_settings, bool two_sided) :
	Material(sampler_settings, two_sided) {}

const material::Sample& Constant::sample(const float3& wo, const Renderstate& rs,
										 Sampler_filter /*filter*/, Worker& worker) {
	auto& sample = worker.sample<Sample>();

	sample.set_basis(rs.geo_n, wo);

	sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

	sample.layer_.set(emittance_.radiance(rs.area));

	return sample;
}

float3 Constant::sample_radiance(const float3& /*wi*/, float2 /*uv*/, float area, float /*time*/,
								 Sampler_filter /*filter*/, Worker& /*worker*/) const {
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

size_t Constant::num_bytes() const {
	return sizeof(*this);
}

::light::Emittance& Constant::emittance() {
	return emittance_;
}

}}}
