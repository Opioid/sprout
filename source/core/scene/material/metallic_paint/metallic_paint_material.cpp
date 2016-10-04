#include "metallic_paint_material.hpp"
#include "metallic_paint_sample.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/material/coating/coating.inl"
#include "scene/shape/geometry/hitpoint.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace metallic_paint {

Material::Material(Sample_cache<Sample>& cache,
				   const Sampler_settings& sampler_settings, bool two_sided) :
	material::Typed_material<Sample_cache<Sample>>(cache, sampler_settings, two_sided) {}

const material::Sample& Material::sample(float3_p wo, const Renderstate& rs,
										 const Worker& worker, Sampler_filter filter) {
	auto& sample = cache_.get(worker.id());

	sample.set_basis(rs.geo_n, wo);
/*
	if (normal_map_) {
		auto& sampler = worker.sampler(sampler_key_, filter);

		float3 nm = sampler.sample_3(*normal_map_, rs.uv);
		float3 n  = math::normalized(rs.tangent_to_world(nm));
		sample.layer_.set_basis(rs.t, rs.b, n);
	} else {*/
		sample.base_.set_basis(rs.t, rs.b, rs.n);

		sample.coating_.set_basis(rs.t, rs.b, rs.n);
//	}

	auto& sampler = worker.sampler(sampler_key_, filter);

	if (flakes_normal_map_.is_valid()) {
		float3 nm = flakes_normal_map_.sample_3(sampler, rs.uv);
		float3 n = math::normalized(rs.tangent_to_world(nm));

		sample.flakes_.set_basis(rs.t, rs.b, n);
	} else {
		sample.flakes_.set_basis(rs.t, rs.b, rs.n);
	}

	sample.base_.set(color_a_, color_b_, a2_);

	float flakes_weight;
	if (flakes_mask_.is_valid()) {
		flakes_weight = flakes_mask_.sample_1(sampler, rs.uv);
	} else {
		flakes_weight = 1.f;
	}

//	sample.flakes_.weight = 0.f;// - math::dot(sample.base_.n, sample.flakes_.n);

	sample.flakes_.set(flakes_ior_, flakes_absorption_, flakes_a2_, flakes_weight);

	sample.coating_.set_color_and_weight(coating_.color, coating_.weight);

	sample.coating_.set(coating_.f0, coating_.a2);

	return sample;
}

size_t Material::num_bytes() const {
	return sizeof(*this);
}

void Material::set_color(float3_p a, float3_p b) {
	color_a_ = a;
	color_b_ = b;
}

void Material::set_roughness(float roughness) {
	a2_ = math::pow4(roughness);
}


void Material::set_flakes_mask(const Texture_adapter& mask) {
	flakes_mask_ = mask;
}

void Material::set_flakes_normal_map(const Texture_adapter& normal_map) {
	flakes_normal_map_ = normal_map;
}

void Material::set_flakes_ior(float3_p ior) {
	flakes_ior_ = ior;
}

void Material::set_flakes_absorption(float3_p absorption) {
	flakes_absorption_ = absorption;
}

void Material::set_flakes_roughness(float roughness) {
	flakes_a2_ = math::pow4(roughness);
}

void Material::set_coating_weight(float weight) {
	coating_.weight = weight;
}

void Material::set_coating_color(float3_p color) {
	coating_.color = color;
}

void Material::set_clearcoat(float ior, float roughness) {
	coating_.f0 = fresnel::schlick_f0(1.f, ior);
	coating_.a2 = math::pow4(roughness);
}

}}}
