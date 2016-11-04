#pragma once

#include "core/scene/material/material.hpp"
#include "core/scene/material/material_sample_cache.hpp"
#include "core/scene/material/light/light_material_sample.hpp"

namespace procedural { namespace sky {

class Model;

class Material : public scene::material::Typed_material<
		scene::material::Sample_cache<scene::material::light::Sample>> {

public:

	using Sampler_filter = scene::material::Sampler_settings::Filter;

	Material(scene::material::Sample_cache<scene::material::light::Sample>& cache,
			 Model& model);

protected:

	Model& model_;
};

}}
