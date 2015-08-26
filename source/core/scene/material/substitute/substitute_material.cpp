#include "substitute_material.hpp"
#include "substitute_sample.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "scene/material/material_sample_cache.inl"
#include "scene/shape/geometry/differential.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace substitute {

Substitute::Substitute(Generic_sample_cache<Sample>& cache, std::shared_ptr<image::texture::Texture_2D> mask) :
	Material(cache, mask) {}

const material::Sample& Substitute::sample(const shape::Differential& dg, const math::float3& wo,
										   const image::texture::sampler::Sampler_2D& sampler,
										   uint32_t worker_id) {
	auto& sample = cache_.get(worker_id);

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
		sample.set(color, emission, surface.x, surface.y);
	} else {
		sample.set(color, surface.x, surface.y);
	}

	return sample;
}

math::float3 Substitute::sample_emission(math::float2 uv, const image::texture::sampler::Sampler_2D& sampler) const {
	if (emission_map_) {
		return emission_factor_ * sampler.sample_3(*emission_map_, uv);
	} else {
		return math::float3::identity;
	}
}

math::float3 Substitute::average_emission() const {
	if (emission_map_) {
		return emission_factor_ * emission_map_->average().xyz;
	} else {
		return math::float3::identity;
	}
}

const image::texture::Texture_2D* Substitute::emission_map() const {
	return emission_map_.get();
}

void Substitute::set_color_map(std::shared_ptr<image::texture::Texture_2D> color_map) {
	color_map_ = color_map;
}

void Substitute::set_normal_map(std::shared_ptr<image::texture::Texture_2D> normal_map) {
	normal_map_ = normal_map;
}

void Substitute::set_surface_map(std::shared_ptr<image::texture::Texture_2D> surface_map) {
	surface_map_ = surface_map;
}

void Substitute::set_emission_map(std::shared_ptr<image::texture::Texture_2D> emission_map) {
	emission_map_ = emission_map;
}

void Substitute::set_color(const math::float3& color) {
	color_ = color;
}

void Substitute::set_roughness(float roughness) {
	roughness_ = roughness;
}

void Substitute::set_metallic(float metallic) {
	metallic_ = metallic;
}

void Substitute::set_emission_factor(float emission_factor) {
	emission_factor = emission_factor;
}

void Substitute::set_two_sided(bool two_sided) {
	two_sided_ = two_sided;
}

}}}
