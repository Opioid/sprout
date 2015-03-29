#pragma once

#include "surrounding.hpp"
#include <memory>

namespace image { namespace texture {

class Texture;

}}

namespace scene { namespace surrounding {

class Sphere : public Surrounding {
public:

	Sphere(std::shared_ptr<image::texture::Texture> texture);

	virtual math::float3 sample(const math::Oray& ray) const;

private:

};

}}
