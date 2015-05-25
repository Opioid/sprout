#pragma once

#include "scene/shape/geometry/shape_intersection.hpp"

namespace image { namespace sampler {

class Sampler_2D;

}}

namespace scene {

namespace material {

class IMaterial;

}

class Prop;

struct Intersection {
	material::IMaterial* material() const;

	float opacity(const image::sampler::Sampler_2D& sampler) const;

	Prop* prop;
	shape::Intersection geo;
};

}
