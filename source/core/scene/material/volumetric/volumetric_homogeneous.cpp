#include "volumetric_homogeneous.hpp"
#include "scene/material/material_sample_helper.hpp"
#include "base/math/ray.inl"

namespace scene::material::volumetric {

Homogeneous::Homogeneous(Sampler_settings const& sampler_settings) : Material(sampler_settings) {}

float3 Homogeneous::emission(math::Ray const& /*ray*/, Transformation const& /*transformation*/,
							 float /*step_size*/, rnd::Generator& /*rng*/,
							 Sampler_filter /*filter*/, Worker const& /*worker*/) const {
	return float3::identity();
}

float3 Homogeneous::absorption_coefficient(float2 /*uv*/, Sampler_filter /*filter*/,
										   Worker const& /*worker*/) const {
	return absorption_coefficient_;
}

Material::CC Homogeneous::collision_coefficients(float2 /*uv*/, Sampler_filter /*filter*/,
												 Worker const& /*worker*/) const {
	return {absorption_coefficient_, scattering_coefficient_};
}

Material::CC Homogeneous::collision_coefficients(f_float3 /*p*/, Sampler_filter /*filter*/,
												 Worker const& /*worker*/) const {
	return {absorption_coefficient_, scattering_coefficient_};
}

float Homogeneous::majorant_mu_t() const {
	return math::max_component(absorption_coefficient_ + scattering_coefficient_);
}

size_t Homogeneous::num_bytes() const {
	return sizeof(*this);
}

}
