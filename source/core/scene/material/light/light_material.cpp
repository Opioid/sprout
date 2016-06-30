#include "light_material.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace light {

Material::Material(Generic_sample_cache<Sample>& cache,
				   const Sampler_settings& sampler_settings, bool two_sided) :
	material::Typed_material<Generic_sample_cache<Sample>>(cache, sampler_settings, two_sided) {}

}}}
