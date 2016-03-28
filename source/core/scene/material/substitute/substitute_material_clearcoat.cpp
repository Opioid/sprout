#include "substitute_material_clearcoat.hpp"
#include "substitute_sample_clearcoat.hpp"
#include "image/texture/texture_2d.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "scene/scene_worker.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_sample_cache.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/shape/geometry/differential.inl"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace substitute {

Material_clearcoat::Material_clearcoat(Generic_sample_cache<Sample_clearcoat>& cache,
									   std::shared_ptr<image::texture::Texture_2D> mask,
									   const Sampler_settings& sampler_settings, bool two_sided) :
	material::Typed_material<Generic_sample_cache<Sample_clearcoat>>(cache, mask, sampler_settings, two_sided) {}

const material::Sample& Material_clearcoat::sample(const shape::Differential& dg, const math::float3& wo,
												   float /*time*/, float /*ior_i*/,
												   const Worker& worker, Sampler_settings::Filter filter) {
	auto& sample = cache_.get(worker.id());

	auto& sampler = worker.sampler(sampler_key_, filter);

	if (normal_map_) {
		math::float3 nm = sampler.sample_3(*normal_map_, dg.uv);
		math::float3 n = math::normalized(dg.tangent_to_world(nm));

		sample.set_basis(dg.t, dg.b, n, dg.geo_n, wo, two_sided_);
	} else {
		sample.set_basis(dg.t, dg.b, dg.n, dg.geo_n, wo, two_sided_);
	}

	math::float3 color;

	if (color_map_) {
		color = sampler.sample_3(*color_map_, dg.uv);
	} else {
		color = color_;
	}

	math::float2 surface;

	if (surface_map_) {
		surface  = sampler.sample_2(*surface_map_, dg.uv);
	} else {
		surface.x = roughness_;
		surface.y = metallic_;
	}

	if (emission_map_) {
		math::float3 emission = emission_factor_ * sampler.sample_3(*emission_map_, dg.uv);
		sample.set(color, emission, constant_f0_, surface.x, surface.y);
	} else {
		sample.set(color, math::float3_identity, constant_f0_, surface.x, surface.y);
	}

	return sample;
}

math::float3 Material_clearcoat::sample_emission(math::float2 uv, float /*time*/,
									   const Worker& worker, Sampler_settings::Filter filter) const {
	if (emission_map_) {
		auto& sampler = worker.sampler(sampler_key_, filter);
		return emission_factor_ * sampler.sample_3(*emission_map_, uv);
	} else {
		return math::float3_identity;
	}
}

math::float3 Material_clearcoat::average_emission() const {
	if (emission_map_) {
		return emission_factor_ * emission_map_->average_3();
	} else {
		return math::float3_identity;
	}
}

bool Material_clearcoat::has_emission_map() const {
	return nullptr != emission_map_;
}

void Material_clearcoat::set_color_map(std::shared_ptr<image::texture::Texture_2D> color_map) {
	color_map_ = color_map;
}

void Material_clearcoat::set_normal_map(std::shared_ptr<image::texture::Texture_2D> normal_map) {
	normal_map_ = normal_map;
}

void Material_clearcoat::set_surface_map(std::shared_ptr<image::texture::Texture_2D> surface_map) {
	surface_map_ = surface_map;
}

void Material_clearcoat::set_emission_map(std::shared_ptr<image::texture::Texture_2D> emission_map) {
	emission_map_ = emission_map;
}

void Material_clearcoat::set_color(const math::float3& color) {
	color_ = color;
}

void Material_clearcoat::set_ior(float ior) {
	constant_f0_ = fresnel::schlick_f0(1.f, ior);
}

void Material_clearcoat::set_roughness(float roughness) {
	roughness_ = roughness;
}

void Material_clearcoat::set_metallic(float metallic) {
	metallic_ = metallic;
}

void Material_clearcoat::set_emission_factor(float emission_factor) {
	emission_factor_ = emission_factor;
}

}}}
