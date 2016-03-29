#pragma once

#include "substitute_material_base.hpp"

namespace scene { namespace material { namespace substitute {

class Sample_clearcoat;

class Material_clearcoat : public Material_base<Sample_clearcoat> {
public:

	Material_clearcoat(Generic_sample_cache<Sample_clearcoat>& cache, std::shared_ptr<image::texture::Texture_2D> mask,
					   const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(const shape::Differential& dg, const math::float3& wo,
										   float time, float ior_i,
										   const Worker& worker, Sampler_settings::Filter filter) final override;

private:

};

}}}
