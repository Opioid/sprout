#pragma once

#include "scene/entity/entity.hpp"
#include "base/math/ray.hpp"
#include <memory>

namespace scene {

namespace shape {

struct Intersection;
class Shape;

}

class Prop : public Entity {
public:

	void init(std::shared_ptr<shape::Shape> shape);

	bool intersect(math::Oray& ray, shape::Intersection& intersection) const;

	bool intersect_p(math::Oray& ray) const;

private:

	std::shared_ptr<shape::Shape> shape_;

};

}
