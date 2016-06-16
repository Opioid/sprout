#pragma once

#include "substitute_base_material.hpp"
#include "scene/material/coating/coating.hpp"

namespace scene { namespace material { namespace substitute {

class Sample_clearcoat;

class Material_clearcoat : public Material_base<Sample_clearcoat> {

public:

	Material_clearcoat(Generic_sample_cache<Sample_clearcoat>& cache,
					   std::shared_ptr<image::texture::Texture_2D> mask,
					   const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(const shape::Hitpoint& hp, math::pfloat3 wo,
										   float area, float time, float ior_i,
										   const Worker& worker,
										   Sampler_filter filter) final override;

	void set_clearcoat(float ior, float roughness, float weight);

private:

	coating::Clearcoat clearcoat_;
};

}}}
