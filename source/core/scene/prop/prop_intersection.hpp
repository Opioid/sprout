#pragma once

#include "scene/shape/geometry/shape_intersection.hpp"

namespace scene {

class Prop;

struct Intersection {
	Prop* prop;
	shape::Intersection geo;
};

}
