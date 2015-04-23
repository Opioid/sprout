#pragma once

#include "scene/entity/entity.hpp"
#include "scene/shape/node_stack.hpp"
#include "base/math/ray.hpp"
#include "base/math/bounding/aabb.hpp"
#include <memory>
#include <vector>

namespace scene {

namespace shape {

struct Intersection;
class Shape;

}

namespace material {

class IMaterial;

}

class Prop : public Entity {
public:

	typedef std::vector<std::shared_ptr<material::IMaterial>> Materials;

	virtual ~Prop();

	void init(std::shared_ptr<shape::Shape> shape, const Materials& materials);

	bool intersect(math::Oray& ray, Node_stack& node_stack, shape::Intersection& intersection) const;

	bool intersect_p(const math::Oray& ray, Node_stack& node_stack) const;

	const shape::Shape* shape() const;

	const math::AABB& aabb() const;

	material::IMaterial& material(uint32_t index) const;

private:

	virtual void on_set_transformation();

	std::shared_ptr<shape::Shape> shape_;

	math::AABB aabb_;

	Materials materials_;
};

}
