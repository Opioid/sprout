#pragma once

#include "surrounding.hpp"
#include "image/texture/texture_2d.hpp"
#include "image/texture/sampler/sampler_spherical_nearest.hpp"
#include <memory>

namespace image {

class Image;

}

namespace scene { namespace surrounding {

class Sphere : public Surrounding {
public:

	Sphere(std::shared_ptr<image::Image> image);

	virtual math::float3 sample(const math::Oray& ray) const;

private:

	image::Texture_2D texture_;
	image::sampler::Spherical_nearest sampler_nearest_;
};

}}
