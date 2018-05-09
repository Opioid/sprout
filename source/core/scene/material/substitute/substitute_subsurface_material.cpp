#include "substitute_subsurface_material.hpp"
#include "substitute_base_sample.inl"
#include "substitute_base_material.inl"
#include "scene/material/material_attenuation.hpp"
#include "scene/material/volumetric/volumetric_sample.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "base/math/ray.inl"
#include "base/math/vector4.inl"

#include "scene/material/null/null_sample.hpp"

namespace scene::material::substitute {

Material_subsurface::Material_subsurface(Sampler_settings const& sampler_settings) :
	Material_base(sampler_settings, false) {}

const material::Sample& Material_subsurface::sample(f_float3 wo, const Renderstate& rs,
													Sampler_filter filter,
													sampler::Sampler& /*sampler*/,
													Worker const& worker) const {
	if (rs.subsurface) {
		auto& sample = worker.sample<volumetric::Sample>();

		sample.set_basis(rs.geo_n, wo);

//		sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

//		if (color_map_.is_valid()) {
//			auto& sampler = worker.sampler_2D(sampler_key(), filter);
//			float3 const color = color_map_.sample_3(sampler, rs.uv);

//			float3 absorption_coefficient;
//			float3 scattering_coefficient;

//			attenuation(color, attenuation_distance_,
//						absorption_coefficient, scattering_coefficient);

//			sample.set(absorption_coefficient, scattering_coefficient, anisotropy_);
//		} else {
			sample.set(anisotropy_);
//		}

		return sample;
	}

	auto& sample = worker.sample<Sample_subsurface>();

	auto& sampler = worker.sampler_2D(sampler_key(), filter);

	set_sample(wo, rs, sampler, sample);

	sample.set(anisotropy_, sior_);

	return sample;

//	auto& sample = worker.sample<null::Sample>();

//	sample.set_basis(rs.geo_n, wo);

//	sample.set(absorption_coefficient_, scattering_coefficient_, anisotropy_);

//	return sample;
}

size_t Material_subsurface::num_bytes() const {
	return sizeof(*this);
}

void Material_subsurface::set_attenuation(float3 const& absorption_color,
										  float3 const& scattering_color,
										  float distance) {
	absorption_color_ = absorption_color;

	attenuation(absorption_color, scattering_color, distance,
				absorption_coefficient_, scattering_coefficient_);

	attenuation_distance_ = distance;
}

void Material_subsurface::set_volumetric_anisotropy(float anisotropy) {
	anisotropy_ = std::clamp(anisotropy, -0.999f, 0.999f);
}

void Material_subsurface::set_ior(float ior, float external_ior) {
	Material_base::set_ior(ior, external_ior);

	sior_.ior_i_ = ior;
	sior_.ior_o_ = external_ior;
	sior_.eta_i_ = external_ior / ior;
	sior_.eta_t_ = ior / external_ior;
}

float3 Material_subsurface::emission(math::Ray const& /*ray*/,
									 Transformation const& /*transformation*/, float /*step_size*/,
									 rnd::Generator& /*rng*/, Sampler_filter /*filter*/,
									 Worker const& /*worker*/) const {
	return float3::identity();
}

float3 Material_subsurface::absorption_coefficient(float2 uv, Sampler_filter filter,
												   Worker const& worker) const {
	if (color_map_.is_valid()) {
		auto& sampler = worker.sampler_2D(sampler_key(), filter);
		float3 const color = color_map_.sample_3(sampler, uv);

		return extinction_coefficient(color, attenuation_distance_);
	}

	return absorption_coefficient_;
}

Material::CE Material_subsurface::collision_coefficients(float2 uv, Sampler_filter filter,
												 Worker const& worker) const {
	if (color_map_.is_valid()) {
		auto& sampler = worker.sampler_2D(sampler_key(), filter);
		float3 const color = color_map_.sample_3(sampler, uv);

		float3 mu_a, mu_s;
		attenuation(color, attenuation_distance_, mu_a, mu_s);

		return {mu_a, mu_s};
	}

	return {absorption_coefficient_, scattering_coefficient_};
}

Material::CE Material_subsurface::collision_coefficients(f_float3 /*p*/,
														 Transformation const& /*transformation*/,
														 Sampler_filter /*filter*/,
														 Worker const& /*worker*/) const {
	return {absorption_coefficient_, scattering_coefficient_};
}

Material::CE Material_subsurface::collision_coefficients(f_float3 /*p*/, Sampler_filter /*filter*/,
														 Worker const& /*worker*/) const {
	return {absorption_coefficient_, scattering_coefficient_};
}

size_t Material_subsurface::sample_size() {
	return sizeof(Sample_subsurface);
}

}
