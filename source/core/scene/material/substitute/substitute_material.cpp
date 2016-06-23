#include "substitute_material.hpp"
#include "substitute_sample.hpp"
#include "substitute_base_material.inl"

namespace scene { namespace material { namespace substitute {

Material::Material(Generic_sample_cache<Sample>& cache,
				   std::shared_ptr<image::texture::Texture_2D> mask,
				   const Sampler_settings& sampler_settings, bool two_sided) :
	Material_base<Sample>(cache, mask, sampler_settings, two_sided) {}

const material::Sample& Material::sample(const shape::Hitpoint& hp, float3_p wo,
										 float /*area*/, float /*time*/, float /*ior_i*/,
										 const Worker& worker, Sampler_filter filter) {
	auto& sample = cache_.get(worker.id());

	auto& sampler = worker.sampler(sampler_key_, filter);

	set_sample(hp, wo, sampler, sample);

	return sample;
}

}}}
