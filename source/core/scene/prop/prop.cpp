#include "prop.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/shape/shape.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"

namespace scene {

Prop::~Prop() {}

void Prop::init(std::shared_ptr<shape::Shape> shape) {
	shape_ = shape;
}

bool Prop::intersect(math::Oray& ray, shape::Intersection& intersection) const {
	Composed_transformation transformation;
	transformation_at(ray.time, transformation);

	math::float2 bounds;

	if (shape_->is_complex()) {
		if (!aabb_.intersect_p(ray)) {
			return false;
		}
	}

	float hit_t;
	bool hit = shape_->intersect(transformation, ray, bounds, intersection, hit_t);

	if (hit) {
		ray.max_t = hit_t;
		return true;
	}

	return false;
}

bool Prop::intersect_p(const math::Oray& ray) const {
	Composed_transformation transformation;
	transformation_at(ray.time, transformation);

	math::float2 bounds;

	if (shape_->is_complex()) {
		if (!aabb_.intersect_p(ray)) {
			return false;
		}
	}

	return shape_->intersect_p(transformation, ray, bounds);
}

const shape::Shape* Prop::shape() const {
	return shape_.get();
}

const math::AABB& Prop::aabb() const {
	return aabb_;
}

rendering::material::Material* Prop::material(uint32_t index) const {
	return materials_[index].get();
}

void Prop::on_set_transformation() {
	shape_->aabb().transform(transformation_.object_to_world, aabb_);
}

}
