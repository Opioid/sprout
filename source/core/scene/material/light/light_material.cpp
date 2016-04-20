#include "light_material.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace light {

Material::Material(Generic_sample_cache<Sample>& cache,
				   std::shared_ptr<image::texture::Texture_2D> mask,
				   const Sampler_settings& sampler_settings, bool two_sided) :
	material::Typed_material<Generic_sample_cache<Sample>>(cache, mask,
														   sampler_settings,
														   two_sided) {}

}}}
