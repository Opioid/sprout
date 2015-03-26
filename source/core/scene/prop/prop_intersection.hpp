#pragma once

#include "scene/shape/geometry/shape_intersection.hpp"


namespace scene {

namespace material {

class IMaterial;

}

class Prop;

struct Intersection {
	material::IMaterial& material() const;

	Prop* prop;
	shape::Intersection geo;
};

}
