#pragma once

#include "scene/entity/entity.hpp"
#include "base/math/ray.hpp"
#include "base/math/bounding/aabb.hpp"
#include <memory>

namespace scene {

namespace shape {

struct Intersection;
class Shape;

}

class Prop : public Entity {
public:

	virtual ~Prop();

	void init(std::shared_ptr<shape::Shape> shape);

	bool intersect(math::Oray& ray, shape::Intersection& intersection) const;

	bool intersect_p(const math::Oray& ray) const;

	const shape::Shape* shape() const;

	const math::AABB& aabb() const;

private:

	virtual void on_set_transformation();

	std::shared_ptr<shape::Shape> shape_;

	math::AABB aabb_;
};

}
