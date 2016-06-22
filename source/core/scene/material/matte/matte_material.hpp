#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"

namespace scene { namespace material { namespace matte {

class Sample;

class Material : public material::Typed_material<Generic_sample_cache<Sample>> {

public:

	Material(Generic_sample_cache<Sample>& cache,
			 std::shared_ptr<image::texture::Texture_2D> mask,
			 const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(const shape::Hitpoint& hp, math::pfloat3 wo,
										   float area, float time, float ior_i,
										   const Worker& worker,
										   Sampler_filter filter) final override;

	void set_color(math::pfloat3 color);

private:

	math::float3 color_;
};

}}}
