#include "sky_material_base.hpp"
#include "sky_model.hpp"
#include "base/math/vector.inl"

namespace procedural { namespace sky {

using namespace scene::material;

Material::Material(Sample_cache& sample_cache, Model& model) :
	scene::material::Material(sample_cache,
							  Sampler_settings(Sampler_settings::Filter::Linear,
											   Sampler_settings::Address::Repeat,
											   Sampler_settings::Address::Clamp),
							  false),
	model_(model) {}

}}
