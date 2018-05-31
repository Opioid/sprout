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

Material::CC Density::collision_coefficients(float2 /*uv*/, Sampler_filter /*filter*/,
											 Worker const& /*worker*/) const {

	return {absorption_coefficient_, scattering_coefficient_};
}

Material::CC Density::collision_coefficients(f_float3 uvw, Sampler_filter filter,
											 Worker const& worker) const {
	float const d = density(uvw, filter, worker);

	return {d * absorption_coefficient_, d * scattering_coefficient_};
}

}
