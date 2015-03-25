#pragma once

#include "scene/shape/geometry/shape_intersection.hpp"

namespace rendering { namespace material {

class Material;

}}

namespace scene {

class Prop;

struct Intersection {
	rendering::material::Material* material() const;

	Prop* prop;
	shape::Intersection geo;
};

}
