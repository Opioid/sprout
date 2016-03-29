#include "substitute_material.hpp"
#include "substitute_sample.hpp"
#include "substitute_material_base.inl"

namespace scene { namespace material { namespace substitute {

Material::Material(Generic_sample_cache<Sample>& cache,
				   std::shared_ptr<image::texture::Texture_2D> mask,
				   const Sampler_settings& sampler_settings, bool two_sided) :
	Material_base<Sample>(cache, mask, sampler_settings, two_sided) {}

const material::Sample& Material::sample(const shape::Differential& dg, const math::float3& wo,
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

}}}
