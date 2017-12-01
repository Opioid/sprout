#include "body.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/shape/shape.hpp"
#include "scene/shape/morphable_shape.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/quaternion.inl"
#include "base/math/transformation.inl"

namespace scene::body {

Body::~Body() {}

void Body::set_shape(const std::shared_ptr<shape::Shape>& shape) {
	shape_ = shape;

	properties_.clear();
	properties_.set(Properties::Visible_in_camera);
	properties_.set(Properties::Visible_in_reflection);
	properties_.set(Properties::Visible_in_shadow);
}

void Body::morph(thread::Pool& pool) {
	/*if (properties_.test(Properties::Animated))*/ {
		shape::Morphable_shape* morphable = shape_->morphable_shape();
		if (morphable) {
			morphable->morph(local_frame_a_.morphing.targets[0],
							 local_frame_a_.morphing.targets[1],
							 local_frame_a_.morphing.weight, pool);
		}
	}
}

bool Body::intersect(Ray& ray, shape::Node_stack& node_stack,
					 shape::Intersection& intersection) const {
	if (!visible(ray.depth)) {
		return false;
	}

	if (shape_->is_complex() && !aabb_.intersect_p(ray)) {
		return false;
	}

	entity::Composed_transformation temp;
	const auto& transformation = transformation_at(ray.time, temp);

	return shape_->intersect(transformation, ray, node_stack, intersection);
}

bool Body::intersect(Ray& ray, shape::Node_stack& node_stack, float& epsilon) const {
	if (!visible(ray.depth)) {
		return false;
	}

	if (shape_->is_complex() && !aabb_.intersect_p(ray)) {
		return false;
	}

	entity::Composed_transformation temp;
	const auto& transformation = transformation_at(ray.time, temp);

	return shape_->intersect(transformation, ray, node_stack, epsilon);
}

bool Body::intersect_p(const Ray& ray, shape::Node_stack& node_stack) const {
	if (!visible_in_shadow()) {
		return false;
	}

	if (shape_->is_complex() && !aabb_.intersect_p(ray)) {
		return false;
	}

	entity::Composed_transformation temp;
	const auto& transformation = transformation_at(ray.time, temp);

	return shape_->intersect_p(transformation, ray, node_stack);
}

//bool Prop::intersect_p(VVector ray_origin, VVector ray_direction,
//					   VVector ray_inv_direction, VVector ray_min_t, VVector ray_max_t,
//					   float ray_time, shape::Node_stack& node_stack) const {
//	if (!visible_in_shadow()) {
//		return false;
//	}

//	if (shape_->is_complex()
//	&& !aabb_.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
//		return false;
//	}

//	entity::Composed_transformation temp;
//	const auto& transformation = transformation_at(ray_time, temp);

//	return shape_->intersect_p(ray_origin, ray_direction, ray_min_t, ray_max_t,
//							   transformation, node_stack);
//}

const shape::Shape* Body::shape() const {
	return shape_.get();
}

shape::Shape* Body::shape() {
	return shape_.get();
}

const math::AABB& Body::aabb() const {
	return aabb_;
}

bool Body::visible(uint32_t ray_depth) const {
	if (0 == ray_depth) {
		if (!properties_.test(Properties::Visible_in_camera)) {
			return false;
		}
	} else {
		if (!properties_.test(Properties::Visible_in_reflection)) {
			return false;
		}
	}

	return true;
}

void Body::on_set_transformation() {
	if (properties_.test(Properties::Animated)) {
		math::AABB aabb = shape_->transformed_aabb(world_frame_a_);

		constexpr uint32_t num_steps = 3;
		constexpr float interval = 1.f / static_cast<float>(num_steps + 1);
		float t = interval;
		for (uint32_t i = num_steps; i > 0; --i, t += interval) {
			const math::Transformation interpolated = math::lerp(world_frame_a_, world_frame_b_, t);
			const math::AABB tmp = shape_->transformed_aabb(interpolated);
			aabb.merge_assign(tmp);
		}

		const math::AABB tmp = shape_->transformed_aabb(world_frame_b_);
		aabb_ = aabb.merge(tmp);
	} else {
		aabb_ = shape_->transformed_aabb(world_transformation_.object_to_world, world_frame_a_);
	}
}

}
