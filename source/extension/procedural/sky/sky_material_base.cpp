#include "sky_material_base.hpp"
#include "sky_model.hpp"

namespace procedural { namespace sky {

Material::Material(scene::material::Generic_sample_cache<scene::material::light::Sample>& cache,
				   Model& model) :
	scene::material::Typed_material<
		scene::material::Generic_sample_cache<scene::material::light::Sample>>(
			cache, scene::material::Sampler_settings(), false),
	model_(model) {}

}}
