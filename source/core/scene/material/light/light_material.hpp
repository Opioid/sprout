#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"

namespace scene { namespace material { namespace light {

class Sample;

class Material : public material::Typed_material<Generic_sample_cache<Sample>> {
public:

	Material(Generic_sample_cache<Sample>& cache, std::shared_ptr<image::texture::Texture_2D> mask,
			 const Sampler_settings& sampler_settings, bool two_sided);
};

}}}
