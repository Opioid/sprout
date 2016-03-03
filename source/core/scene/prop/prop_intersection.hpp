#pragma once

#include "scene/shape/geometry/shape_intersection.hpp"

namespace image { namespace texture { namespace sampler { class Sampler_2D; }}}

namespace scene { namespace material { class Material; }

class Prop;

struct Intersection {
	material::Material* material() const;

	float opacity(float time, const image::texture::sampler::Sampler_2D& sampler) const;

	const Prop* prop;
	shape::Intersection geo;
};

}
