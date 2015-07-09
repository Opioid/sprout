#include "prop.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/shape/shape.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/bounding/aabb.inl"

namespace scene {

Prop::~Prop() {}

void Prop::init(std::shared_ptr<shape::Shape> shape, const material::Materials& materials) {
	shape_ = shape;
	materials_ = materials;

	has_masked_material_ = false;
	for (auto m : materials_) {
		if (m->is_masked()) {
			has_masked_material_ = true;
			break;
		}
	}
}

bool Prop::intersect(math::Oray& ray, shape::Node_stack& node_stack, shape::Intersection& intersection) const {
	entity::Composed_transformation transformation;
	bool animated = transformation_at(ray.time, transformation);

	math::float2 bounds;

	if (shape_->is_complex()) {
		math::aabb aabb;
		if (animated) {
			shape_->aabb().transform(transformation.object_to_world, aabb);
		} else {
			aabb = aabb_;
		}

		if (!aabb_.intersect_p(ray)) {
			return false;
		}
	}

	return shape_->intersect(transformation, ray, bounds, node_stack, intersection);
}

bool Prop::intersect_p(const math::Oray& ray, shape::Node_stack& node_stack) const {
	entity::Composed_transformation transformation;
	bool animated = transformation_at(ray.time, transformation);

	math::float2 bounds;

	if (shape_->is_complex()) {
		math::aabb aabb;
		if (animated) {
			shape_->aabb().transform(transformation.object_to_world, aabb);
		} else {
			aabb = aabb_;
		}

		if (!aabb_.intersect_p(ray)) {
			return false;
		}
	}

	return shape_->intersect_p(transformation, ray, bounds, node_stack);
}

float Prop::opacity(const math::Oray& ray, shape::Node_stack& node_stack, const image::sampler::Sampler_2D& sampler) const {
	if (!has_masked_material()) {
		return intersect_p(ray, node_stack) ? 1.f : 0.f;
	}

	entity::Composed_transformation transformation;
	bool animated = transformation_at(ray.time, transformation);

	math::float2 bounds;

	if (shape_->is_complex()) {
		math::aabb aabb;
		if (animated) {
			shape_->aabb().transform(transformation.object_to_world, aabb);
		} else {
			aabb = aabb_;
		}

		if (!aabb_.intersect_p(ray)) {
			return 0.f;
		}
	}

	return shape_->opacity(transformation, ray, bounds, node_stack, materials_, sampler);
}

const shape::Shape* Prop::shape() const {
	return shape_.get();
}

shape::Shape* Prop::shape() {
	return shape_.get();
}

const math::aabb& Prop::aabb() const {
	return aabb_;
}

material::IMaterial* Prop::material(uint32_t index) const {
	return materials_[index].get();
}

bool Prop::has_masked_material() const {
	return has_masked_material_;
}

bool Prop::has_emission_mapped_material() const {
	for (auto m : materials_) {
		if (m->emission_map()) {
			return true;
		}
	}

	return false;
}

void Prop::on_set_transformation() {
	if (animated_) {
		entity::Composed_transformation t;

		math::aabb a;
		t.set(world_transformation_a_);
		shape_->aabb().transform(t.object_to_world, a);

		math::aabb b;
		t.set(world_transformation_b_);
		shape_->aabb().transform(t.object_to_world, b);

		aabb_ = a.merge(b);
	} else {
		shape_->aabb().transform(world_transformation_.object_to_world, aabb_);
	}
}

}
