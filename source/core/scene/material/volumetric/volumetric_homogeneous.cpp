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

float3 Homogeneous::optical_depth(const Transformation& /*transformation*/,
								  const math::AABB& /*aabb*/, const math::Ray& ray,
								  float /*step_size*/, rnd::Generator& /*rng*/,
								  Sampler_filter /*filter*/, const Worker& /*worker*/) const {
	return ray.length() * (absorption_coefficient_ + scattering_coefficient_);
}

float3 Homogeneous::absorption(float2 /*uv*/, Sampler_filter /*filter*/,
							   const Worker& /*worker*/) const {
	return absorption_coefficient_;
}

void Homogeneous::extinction(float2 /*uv*/, Sampler_filter /*filter*/, const Worker& /*worker*/,
							 float3& sigma_a, float3& sigma_s) const {
	sigma_a = absorption_coefficient_;
	sigma_s = scattering_coefficient_;
}

void Homogeneous::extinction(const Transformation& /*transformation*/, const float3& /*p*/,
							 Sampler_filter /*filter*/, const Worker& /*worker*/,
							 float3& sigma_a, float3& sigma_s) const {
	sigma_a = absorption_coefficient_;
	sigma_s = scattering_coefficient_;
}

float Homogeneous::majorant_sigma_t() const {
	return math::max_component(absorption_coefficient_ + scattering_coefficient_);
}

size_t Homogeneous::num_bytes() const {
	return sizeof(*this);
}

}
