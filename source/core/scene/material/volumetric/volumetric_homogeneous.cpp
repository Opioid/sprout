#include "volumetric_homogeneous.hpp"
#include "scene/material/material_sample_helper.hpp"
#include "base/math/ray.inl"

namespace scene::material::volumetric {

Homogeneous::Homogeneous(const Sampler_settings& sampler_settings) : Material(sampler_settings) {}

float3 Homogeneous::emission(const Transformation& /*transformation*/, const math::Ray& /*ray*/,
							 float /*step_size*/, rnd::Generator& /*rng*/,
							 Sampler_filter /*filter*/, const Worker& /*worker*/) const {
	return float3::identity();
}

float3 Homogeneous::absorption_coefficient(float2 /*uv*/, Sampler_filter /*filter*/,
										   const Worker& /*worker*/) const {
	return absorption_coefficient_;
}

void Homogeneous::collision_coefficients(float2 /*uv*/, Sampler_filter /*filter*/,
										 const Worker& /*worker*/,
										 float3& mu_a, float3& mu_s) const {
	mu_a = absorption_coefficient_;
	mu_s = scattering_coefficient_;
}

void Homogeneous::collision_coefficients(const Transformation& /*transformation*/,
										 const float3& /*p*/, Sampler_filter /*filter*/,
										 const Worker& /*worker*/,
										 float3& mu_a, float3& mu_s) const {
	mu_a = absorption_coefficient_;
	mu_s = scattering_coefficient_;
}

float Homogeneous::majorant_mu_t() const {
	return math::max_component(absorption_coefficient_ + scattering_coefficient_);
}

size_t Homogeneous::num_bytes() const {
	return sizeof(*this);
}

}
