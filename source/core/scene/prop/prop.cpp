#include "prop.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/shape/shape.hpp"
#include "base/math/vector.inl"

namespace scene {

void Prop::init(std::shared_ptr<shape::Shape> shape) {
	shape_ = shape;
}

bool Prop::intersect(math::Oray& ray, shape::Intersection& intersection) const {
	Composed_transformation transformation;
	transformation_at(ray.time, transformation);

	math::float2 bounding;

	float hit_t;
	bool hit = shape_->intersect(transformation, ray, bounding, intersection, hit_t);

	if (hit) {
		ray.max_t = hit_t;
		return true;
	}

	return false;
}

bool Prop::intersect_p(const math::Oray& ray) const {
	Composed_transformation transformation;
	transformation_at(ray.time, transformation);

	math::float2 bounding;

	return shape_->intersect_p(transformation, ray, bounding);
}

const shape::Shape* Prop::shape() const {
	return shape_.get();
}

const math::AABB& Prop::aabb() const {
	return aabb_;
}

void Prop::on_set_transformation() {
	shape_->aabb().transform(transformation_.object_to_world, aabb_);
}

}
