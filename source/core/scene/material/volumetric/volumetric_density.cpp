#include "volumetric_density.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/material/material_sample_helper.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/ray.inl"
#include "base/random/generator.inl"

namespace scene::material::volumetric {

Density::Density(Sampler_settings const& sampler_settings) : Material(sampler_settings) {}

float3 Density::emission(math::Ray const& /*ray*/, Transformation const& /*transformation*/,
						 float /*step_size*/, rnd::Generator& /*rng*/,
						 Sampler_filter /*filter*/, Worker const& /*worker*/) const {
	return float3::identity();
}

void Density::collision_coefficients(float2 /*uv*/, Sampler_filter /*filter*/,
									 Worker const& /*worker*/, float3& mu_a, float3& mu_s) const {
	mu_a = absorption_coefficient_;
	mu_s = scattering_coefficient_;
}

void Density::collision_coefficients(f_float3 p, Transformation const& transformation,
									 Sampler_filter filter, Worker const& worker,
									 float3& mu_a, float3& mu_s) const {
	float3 const p_o = math::transform_point(p, transformation.world_to_object);

	float const d = density(p_o, filter, worker);

	mu_a = d * absorption_coefficient_;
	mu_s = d * scattering_coefficient_;
}

void Density::collision_coefficients(f_float3 p, Sampler_filter filter, Worker const& worker,
									 float3& mu_a, float3& mu_s) const {
	float const d = density(p, filter, worker);

	mu_a = d * absorption_coefficient_;
	mu_s = d * scattering_coefficient_;
}

}
