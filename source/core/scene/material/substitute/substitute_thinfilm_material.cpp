#include "substitute_thinfilm_material.hpp"
#include "substitute_thinfilm_sample.hpp"
#include "substitute_base_material.inl"

namespace scene { namespace material { namespace substitute {

Material_thinfilm::Material_thinfilm(Generic_sample_cache<Sample_thinfilm>& cache,
									 std::shared_ptr<image::texture::Texture_2D> mask,
									 const Sampler_settings& sampler_settings, bool two_sided) :
	Material_base<Sample_thinfilm>(cache, mask, sampler_settings, two_sided) {}

const material::Sample& Material_thinfilm::sample(const shape::Hitpoint& hp, math::pfloat3 wo,
												  float /*area*/, float /*time*/, float /*ior_i*/,
												  const Worker& worker, Sampler_filter filter) {
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

	math::float3 radiance;
	if (emission_map_) {
		radiance = emission_factor_ * sampler.sample_3(*emission_map_, hp.uv);
	} else {
		radiance = math::float3_identity;
	}

	sample.set(color, radiance, constant_f0_, ior_, surface.x, surface.y,
			   thinfilm_ior_, thinfilm_thickness_);

	return sample;
}

void Material_thinfilm::set_thinfilm(float ior, float thickness) {
	thinfilm_ior_ = ior;
	thinfilm_thickness_ = thickness;
}

}}}
