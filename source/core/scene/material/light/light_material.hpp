#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"

namespace scene { namespace material { namespace light {

class Sample;

class Material : public material::Typed_material<Sample_cache<Sample>> {

public:

	Material(Sample_cache<Sample>& cache,
			 const Sampler_settings& sampler_settings, bool two_sided);
};

}}}
