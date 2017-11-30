#ifndef SU_CORE_SCENE_BODY_BODY_HPP
#define SU_CORE_SCENE_BODY_BODY_HPP

#include "scene/entity/entity.hpp"
#include "base/math/aabb.hpp"
#include <memory>
#include <vector>

namespace thread { class Pool; }

namespace scene {

class Worker;
struct Ray;

namespace shape {

struct Intersection;
class Shape;
class Node_stack;

}

namespace body {

class Body : public entity::Entity {

public:

	virtual ~Body();

	void set_shape(const std::shared_ptr<shape::Shape>& shape);

	void morph(thread::Pool& pool);

	bool intersect(Ray& ray, shape::Node_stack& node_stack,
				   shape::Intersection& intersection) const;

	bool intersect_p(const Ray& ray, shape::Node_stack& node_stack) const;

//	bool intersect_p(VVector ray_origin, VVector ray_direction,
//					 VVector ray_inv_direction, VVector ray_mint_, VVector ray_max_t,
//					 float ray_time, shape::Node_stack& node_stack) const;

	const shape::Shape* shape() const;
	shape::Shape* shape();

	const math::AABB& aabb() const;

protected:

	bool visible(uint32_t ray_depth) const;

	virtual void on_set_transformation() override final;

	// Pre-transformed AABB in world space.
	// For moving objects it must cover the entire area occupied by the object during the tick.
	math::AABB aabb_;

	std::shared_ptr<shape::Shape> shape_;
};

}}

#endif
