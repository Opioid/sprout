#pragma once

#include "scene/entity/entity.hpp"
#include "base/math/ray.hpp"

namespace scene {

namespace shape {

struct Intersection;
class Shape;

}

class Prop : public Entity {
public:

	bool intersect(math::Oray& ray, shape::Intersection& intersection) const;

	bool intersect_p(math::Oray& ray) const;

private:

	shape::Shape* shape_;

};

}
