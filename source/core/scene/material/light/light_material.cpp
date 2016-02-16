#include "light_material.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace light {

Material::Material(Generic_sample_cache<Sample>& cache,
				   std::shared_ptr<image::texture::Texture_2D> mask, bool two_sided) :
	material::Material<Generic_sample_cache<Sample>>(cache, mask, two_sided) {}

}}}
