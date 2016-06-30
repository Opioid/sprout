#include "substitute_coating_material.inl"
#include "substitute_coating_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/material/coating/coating.inl"

namespace scene { namespace material { namespace substitute {

Material_clearcoat::Material_clearcoat(Generic_sample_cache<Sample_clearcoat>& cache,
									   Texture_2D_ptr mask,
									   const Sampler_settings& sampler_settings, bool two_sided) :
	Material_coating<coating::Clearcoat, Sample_clearcoat>(cache, mask, sampler_settings,
														   two_sided) {}

const material::Sample& Material_clearcoat::sample(float3_p wo, const Renderstate& rs,
												   const Worker& worker, Sampler_filter filter) {
	auto& sample = cache_.get(worker.id());

	auto& sampler = worker.sampler(sampler_key_, filter);

	set_sample(wo, rs, sampler, sample);

	set_coating_basis(rs, sampler, sample);

	sample.coating_.set(coating_.color, coating_.f0, coating_.a2);

	return sample;
}

void Material_clearcoat::set_clearcoat(float3_p color, float ior, float roughness, float weight) {
	coating_.color = color;
	coating_.f0 = fresnel::schlick_f0(1.f, ior);
	coating_.a2 = math::pow4(roughness);
	coating_.weight = weight;
}

Material_thinfilm::Material_thinfilm(Generic_sample_cache<Sample_thinfilm>& cache,
									 Texture_2D_ptr mask,
									 const Sampler_settings& sampler_settings, bool two_sided) :
	Material_coating<coating::Thinfilm, Sample_thinfilm>(cache, mask, sampler_settings,
														 two_sided) {}

const material::Sample& Material_thinfilm::sample(float3_p wo, const Renderstate& rs,
												  const Worker& worker, Sampler_filter filter) {
	auto& sample = cache_.get(worker.id());

	auto& sampler = worker.sampler(sampler_key_, filter);

	set_sample(wo, rs, sampler, sample);

	set_coating_basis(rs, sampler, sample);

	sample.coating_.set(coating_.ior, coating_.a2, coating_.thickness);

	return sample;
}

void Material_thinfilm::set_thinfilm(float ior, float roughness, float thickness, float weight) {
	coating_.ior = ior;
	coating_.a2  = math::pow4(roughness);
	coating_.thickness = thickness;
	coating_.weight = weight;
}

}}}
