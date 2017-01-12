#include "sky_material_base.hpp"
#include "sky_model.hpp"
#include "base/math/vector.inl"

namespace procedural { namespace sky {

Material::Material(scene::material::Sample_cache& sample_cache, Model& model) :
	scene::material::Material(sample_cache, scene::material::Sampler_settings(), false),
	model_(model) {}

}}
