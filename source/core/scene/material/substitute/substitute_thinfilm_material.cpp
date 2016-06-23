#include "substitute_thinfilm_material.hpp"
#include "substitute_thinfilm_sample.hpp"
#include "substitute_base_material.inl"

namespace scene { namespace material { namespace substitute {

Material_thinfilm::Material_thinfilm(Generic_sample_cache<Sample_thinfilm>& cache,
									 Texture_2D_ptr mask,
									 const Sampler_settings& sampler_settings, bool two_sided) :
	Material_base<Sample_thinfilm>(cache, mask, sampler_settings, two_sided) {}

const material::Sample& Material_thinfilm::sample(const shape::Hitpoint& hp, float3_p wo,
												  float /*area*/, float /*time*/, float /*ior_i*/,
												  const Worker& worker, Sampler_filter filter) {
	auto& sample = cache_.get(worker.id());

	auto& sampler = worker.sampler(sampler_key_, filter);

	set_sample(hp, wo, sampler, sample);

	sample.set_thinfilm(thinfilm_);

	return sample;
}

void Material_thinfilm::set_thinfilm(float ior, float roughness, float thickness, float weight) {
	thinfilm_.ior = ior;
	thinfilm_.a2  = math::pow4(roughness);
	thinfilm_.thickness = thickness;
	thinfilm_.weight = weight;
}

}}}
