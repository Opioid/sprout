#include "substitute_material_clearcoat.hpp"
#include "substitute_sample_clearcoat.hpp"
#include "substitute_material_base.inl"

namespace scene { namespace material { namespace substitute {

Material_clearcoat::Material_clearcoat(Generic_sample_cache<Sample_clearcoat>& cache,
									   std::shared_ptr<image::texture::Texture_2D> mask,
									   const Sampler_settings& sampler_settings, bool two_sided) :
	Material_base<Sample_clearcoat>(cache, mask, sampler_settings, two_sided) {}

const material::Sample& Material_clearcoat::sample(const shape::Hitpoint& hp, math::pfloat3 wo,
												   float /*time*/, float /*ior_i*/,
												   const Worker& worker, Sampler_settings::Filter filter) {
	auto& sample = cache_.get(worker.id());

	auto& sampler = worker.sampler(sampler_key_, filter);

	if (normal_map_) {
		math::float3 nm = sampler.sample_3(*normal_map_, hp.uv);
		math::float3 n = math::normalized(hp.tangent_to_world(nm));

		sample.set_basis(hp.t, hp.b, n, hp.geo_n, wo, two_sided_);
	} else {
		sample.set_basis(hp.t, hp.b, hp.n, hp.geo_n, wo, two_sided_);
	}

	math::float3 color;

	if (color_map_) {
		color = sampler.sample_3(*color_map_, hp.uv);
	} else {
		color = color_;
	}

	math::float2 surface;

	if (surface_map_) {
		surface = sampler.sample_2(*surface_map_, hp.uv);
		surface.x = math::pow4(surface.x);
	} else {
		surface.x = a2_;
		surface.y = metallic_;
	}

	if (emission_map_) {
		math::float3 radiance = emission_factor_ * sampler.sample_3(*emission_map_, hp.uv);
		sample.set(color, radiance, constant_f0_, surface.x, surface.y, clearcoat_ior_, clearcoat_a2_);
	} else {
		sample.set(color, math::float3_identity, constant_f0_, surface.x, surface.y, clearcoat_ior_, clearcoat_a2_);
	}

	return sample;
}

void Material_clearcoat::set_clearcoat(float ior, float roughness) {
	clearcoat_ior_ = ior;
	clearcoat_a2_  = math::pow4(roughness);
}

}}}
