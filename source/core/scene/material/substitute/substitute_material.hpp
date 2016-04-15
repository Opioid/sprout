#pragma once

#include "substitute_material_base.hpp"

namespace scene { namespace material { namespace substitute {

class Sample;

class Material : public Material_base<Sample> {
public:

	Material(Generic_sample_cache<Sample>& cache,
			 std::shared_ptr<image::texture::Texture_2D> mask,
			 const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(const shape::Hitpoint& hp, math::pfloat3 wo,
										   float time, float ior_i,
										   const Worker& worker,
										   Sampler_settings::Filter filter) final override;
};

}}}
