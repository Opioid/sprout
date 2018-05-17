#include "substitute_subsurface_material.hpp"
#include "substitute_base_sample.inl"
#include "substitute_base_material.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "scene/material/material_attenuation.hpp"
#include "scene/material/volumetric/volumetric_octree_builder.hpp"
#include "scene/material/volumetric/volumetric_sample.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/ray.inl"
#include "base/math/vector4.inl"
#include "base/spectrum/heatmap.hpp"

#include "scene/material/null/null_sample.hpp"

#include <iostream>

namespace scene::material::substitute {

Material_subsurface::Material_subsurface(Sampler_settings const& sampler_settings) :
	Material_base(sampler_settings, false) {}

void Material_subsurface::compile() {
	if (density_map_.is_valid()) {
		auto const& texture = *density_map_.texture();

		const int3 d = texture.dimensions_3();

		float max_density = 0.f;
		for (int32_t i = 0, len = d[0] * d[1] * d[2]; i < len; ++i) {
			max_density = std::max(texture.at_1(i), max_density);
		}

		float3 const extinction_coefficient = absorption_coefficient_ + scattering_coefficient_;

		float const max_extinction = math::max_component(extinction_coefficient);

		majorant_mu_t_ = max_density * max_extinction;

		volumetric::Octree_builder builder;
		builder.build(tree_, texture, max_extinction);
	}

//	attenuation(float3(0.25f), attenuation_distance_,
//				absorption_coefficient_, scattering_coefficient_);

//	float3 const extinction_coefficient = absorption_coefficient_ + scattering_coefficient_;

//	float const max_extinction = math::max_component(extinction_coefficient);

//	majorant_mu_t_ = max_extinction;
}

const material::Sample& Material_subsurface::sample(f_float3 wo, Renderstate const& rs,
													Sampler_filter filter,
													sampler::Sampler& /*sampler*/,
													Worker const& worker) const {
	if (rs.subsurface) {
		auto& sample = worker.sample<volumetric::Sample>();

		sample.set_basis(rs.geo_n, wo);

		sample.set(anisotropy_);

		return sample;
	}

	auto& sample = worker.sample<Sample_subsurface>();

	auto& sampler = worker.sampler_2D(sampler_key(), filter);

	set_sample(wo, rs, sampler, sample);

	sample.set(anisotropy_, sior_);

	return sample;
}

size_t Material_subsurface::num_bytes() const {
	return sizeof(*this);
}

void Material_subsurface::set_density_map(Texture_adapter const& density_map) {
	density_map_ = density_map;
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

Material::CC Material_subsurface::collision_coefficients(float2 uv, Sampler_filter filter,
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

Material::CC Material_subsurface::collision_coefficients(f_float3 p, Sampler_filter filter,
														 Worker const& worker) const {
	SOFT_ASSERT(density_map_.is_valid());

	float const d = density(p, filter, worker);

	return  {d * absorption_coefficient_, d * scattering_coefficient_};

//	float3 p_g = 0.5f * (float3(1.f) + p);

//	float const x = 1.f - (p_g[1] - 0.2f);
//	float const d = std::clamp(x * x, 0.01f, 1.f);

//	float3 const c = color(p, filter, worker);

//	float3 mu_a, mu_s;
//	attenuation(c, attenuation_distance_, mu_a, mu_s);

//	return {d * mu_a, d * mu_s};
}

float Material_subsurface::majorant_mu_t() const {
	return majorant_mu_t_;
}

volumetric::Octree const* Material_subsurface::volume_octree() const {
//	return nullptr;
	return &tree_;
}

bool Material_subsurface::is_heterogeneous_volume() const {
	return density_map_.is_valid();
}

size_t Material_subsurface::sample_size() {
	return sizeof(Sample_subsurface);
}

float Material_subsurface::density(f_float3 p, Sampler_filter filter, Worker const& worker) const {
	// p is in object space already

	float3 p_g = 0.5f * (float3(1.f) + p);

	auto const& sampler = worker.sampler_3D(sampler_key(), filter);

	return density_map_.sample_1(sampler, p_g);
}

float3 Material_subsurface::color(f_float3 p, Sampler_filter filter, Worker const& worker) const {
	float3 p_g = 0.5f * (float3(1.f) + p);

//	auto const& sampler = worker.sampler_3D(sampler_key(), filter);

//	float const d = std::min(16.f * density_map_.sample_1(sampler, p_g), 1.f);

	float const x = 1.f - (p_g[1] - 0.5f);
	float const d = std::clamp(x * x, 0.1f, 1.f);

//	return float3(d);

	return math::max(d * spectrum::heatmap(p_g[0] + 0.15f), 0.25f);
}

}
