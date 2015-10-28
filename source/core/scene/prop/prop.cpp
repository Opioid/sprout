#include "prop.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/shape/shape.hpp"
#include "scene/shape/morphable_shape.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/quaternion.inl"
#include "base/math/bounding/aabb.inl"

namespace scene {

Prop::~Prop() {}

void Prop::init(std::shared_ptr<shape::Shape> shape, const material::Materials& materials) {
	properties_.clear();

	shape_ = shape;
	materials_ = materials;

	for (auto m : materials_) {
		if (m->is_masked()) {
			properties_.set(Properties::Has_masked_material);
			break;
		}
	}
}

void Prop::set_visibility(bool primary, bool secondary) {
	properties_.set(Properties::Primary_visibility,   primary);
	properties_.set(Properties::Secondary_visibility, secondary);
}

bool Prop::intersect(math::Oray& ray, shape::Node_stack& node_stack, shape::Intersection& intersection) const {
	if (!visible(ray.depth)) {
		return false;
	}

	math::float2 bounds;

	if (shape_->is_complex() && !aabb_.intersect_p(ray)) {
		return false;
	}

	entity::Composed_transformation transformation;
	transformation_at(ray.time, transformation);

	return shape_->intersect(transformation, ray, bounds, node_stack, intersection);
}

bool Prop::intersect_p(const math::Oray& ray, shape::Node_stack& node_stack) const {
	if (!visible(ray.depth)) {
		return false;
	}

	math::float2 bounds;

	if (shape_->is_complex() && !aabb_.intersect_p(ray)) {
		return false;
	}

	entity::Composed_transformation transformation;
	transformation_at(ray.time, transformation);

	return shape_->intersect_p(transformation, ray, bounds, node_stack);
}

float Prop::opacity(const math::Oray& ray, shape::Node_stack& node_stack,
					const image::texture::sampler::Sampler_2D& sampler) const {
	if (!visible(ray.depth)) {
		return 0.f;
	}

	if (!has_masked_material()) {
		return intersect_p(ray, node_stack) ? 1.f : 0.f;
	}

	math::float2 bounds;

	if (shape_->is_complex() && !aabb_.intersect_p(ray)) {
		return false;
	}

	entity::Composed_transformation transformation;
	transformation_at(ray.time, transformation);

	return shape_->opacity(transformation, ray, bounds, node_stack, materials_, sampler);
}

const shape::Shape* Prop::shape() const {
	return shape_.get();
}

shape::Shape* Prop::shape() {
	return shape_.get();
}

void Prop::morph(thread::Pool& pool) {
	if (animated_) {
		shape::Morphable_shape* morphable = shape_->morphable_shape();
		if (morphable) {
			morphable->morph(local_frame_a_.morphing.targets[0], local_frame_a_.morphing.targets[1],
							 local_frame_a_.morphing.weight, pool);
		}
	}
}

const math::aabb& Prop::aabb() const {
	return aabb_;
}

const material::Materials& Prop::materials() const {
	return materials_;
}

material::IMaterial* Prop::material(uint32_t index) const {
	return materials_[index].get();
}

bool Prop::has_masked_material() const {
	return properties_.test(Properties::Has_masked_material);
}

bool Prop::primary_visibility() const {
	return properties_.test(Properties::Primary_visibility);
}

bool Prop::secondary_visibility() const {
	return properties_.test(Properties::Secondary_visibility);
}

bool Prop::visible(uint32_t ray_depth) const {
	if (ray_depth < 1) {
		if (!properties_.test(Properties::Primary_visibility)) {
			return false;
		}
	} else {
		if (!properties_.test(Properties::Secondary_visibility)) {
			return false;
		}
	}

	return true;
}

void Prop::on_set_transformation() {
	if (animated_) {
		math::aabb aabb = shape_->aabb().transform(math::float4x4(world_frame_a_));

		constexpr uint32_t num_steps = 3;
		constexpr float interval = 1.f / static_cast<float>(num_steps + 1);
		float t = interval;
		for (uint32_t i = 0; i < num_steps; ++i) {
			math::float4x4 interpolated = math::float4x4(math::lerp(world_frame_a_, world_frame_b_, t));
			math::aabb tmp = shape_->aabb().transform(interpolated);
			aabb.merge_assign(tmp);
			t += interval;
		}

		math::aabb tmp = shape_->aabb().transform(math::float4x4(world_frame_b_));
		aabb_ = aabb.merge(tmp);
	} else {
		aabb_ = shape_->aabb().transform(world_transformation_.object_to_world);
	}
}

}
