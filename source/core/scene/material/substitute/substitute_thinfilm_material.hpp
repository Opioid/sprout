#pragma once

#include "substitute_base_material.hpp"
#include "scene/material/coating/coating.hpp"

namespace scene { namespace material { namespace substitute {

class Sample_thinfilm;

class Material_thinfilm : public Material_base<Sample_thinfilm> {

public:

	Material_thinfilm(Generic_sample_cache<Sample_thinfilm>& cache,
					  std::shared_ptr<image::texture::Texture_2D> mask,
					  const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(const shape::Hitpoint& hp, float3_p wo,
										   float area, float time, float ior_i,
										   const Worker& worker,
										   Sampler_filter filter) final override;

	void set_thinfilm(float ior, float roughness, float thickness, float weight);

private:

	coating::Thinfilm thinfilm_;
};

}}}
