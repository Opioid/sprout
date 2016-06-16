#include "substitute_clearcoat_material.hpp"
#include "substitute_clearcoat_sample.hpp"
#include "substitute_base_material.inl"

namespace scene { namespace material { namespace substitute {

Material_clearcoat::Material_clearcoat(Generic_sample_cache<Sample_clearcoat>& cache,
									   std::shared_ptr<image::texture::Texture_2D> mask,
									   const Sampler_settings& sampler_settings, bool two_sided) :
	Material_base<Sample_clearcoat>(cache, mask, sampler_settings, two_sided) {}

const material::Sample& Material_clearcoat::sample(const shape::Hitpoint& hp, math::pfloat3 wo,
												   float /*area*/, float /*time*/, float /*ior_i*/,
												   const Worker& worker, Sampler_filter filter) {
	auto& sample = cache_.get(worker.id());

	auto& sampler = worker.sampler(sampler_key_, filter);

	set_sample(hp, wo, sampler, sample);

	sample.set_clearcoat(clearcoat_ior_, clearcoat_a2_);

	return sample;
}

void Material_clearcoat::set_clearcoat(float ior, float roughness) {
	clearcoat_ior_ = ior;
	clearcoat_a2_  = math::pow4(roughness);
}

}}}
