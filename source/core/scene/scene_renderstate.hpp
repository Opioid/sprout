#pragma once

#include "shape/geometry/shape_intersection.hpp"

namespace scene {

struct Renderstate {
	shape::Intersection geo;
	float area;
	float time;
	float ior;
};

}
