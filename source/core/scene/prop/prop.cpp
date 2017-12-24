#include "prop.hpp"
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

namespace scene::prop {

Prop::~Prop() {}

void Prop::set_shape(const std::shared_ptr<shape::Shape>& shape) {
	shape_ = shape;

	properties_.clear();
	properties_.set(Properties::Visible_in_camera);
	properties_.set(Properties::Visible_in_reflection);
	properties_.set(Properties::Visible_in_shadow);
}


void Prop::set_shape_and_materials(const std::shared_ptr<shape::Shape>& shape,
								   const material::Materials& materials) {
	set_shape(shape);

	parts_.resize(shape->num_parts());
	for (auto& p : parts_) {
		p.area = 1.f;
		p.light_id = 0xFFFFFFFF;
	}

	materials_ = materials;

	for (auto m : materials_) {
		if (m->is_masked()) {
			properties_.set(Properties::Masked_material);
		}

		if (m->has_tinted_shadow()) {
			properties_.set(Properties::Tinted_shadow);
		}
	}
}


void Prop::morph(thread::Pool& pool) {
	/*if (properties_.test(Properties::Animated))*/ {
		shape::Morphable_shape* morphable = shape_->morphable_shape();
		if (morphable) {
			morphable->morph(local_frame_a_.morphing.targets[0],
							 local_frame_a_.morphing.targets[1],
							 local_frame_a_.morphing.weight, pool);
		}
	}
}

bool Prop::intersect(Ray& ray, shape::Node_stack& node_stack,
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

bool Prop::intersect(Ray& ray, shape::Node_stack& node_stack, float& epsilon, bool& inside) const {
	if (!ray.properties.test(Ray::Property::Recursive)) {
		if (ray.properties.test(Ray::Property::Shadow)) {
			if (!visible_in_shadow()) {
				return false;
			}
		} else if (!visible(ray.depth)) {
			return false;
		}
	}

	if (shape_->is_complex() && !aabb_.intersect_p(ray)) {
		return false;
	}

	entity::Composed_transformation temp;
	const auto& transformation = transformation_at(ray.time, temp);

	return shape_->intersect(transformation, ray, node_stack, epsilon, inside);
}

bool Prop::intersect_p(const Ray& ray, shape::Node_stack& node_stack) const {
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

const shape::Shape* Prop::shape() const {
	return shape_.get();
}

shape::Shape* Prop::shape() {
	return shape_.get();
}

const math::AABB& Prop::aabb() const {
	return aabb_;
}

bool Prop::visible(uint32_t ray_depth) const {
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

void Prop::on_set_transformation() {
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

void Prop::set_parameters(const json::Value& /*parameters*/) {}

void Prop::prepare_sampling(uint32_t part, uint32_t light_id,
							bool material_importance_sampling, thread::Pool& pool) {
	shape_->prepare_sampling(part);

	entity::Composed_transformation temp;
	const auto& transformation = transformation_at(0.f, temp);

	const float area = shape_->area(part, transformation.scale);
	parts_[part].area = area;

	parts_[part].light_id = light_id;

	materials_[part]->prepare_sampling(*shape_, part, transformation, area,
									   material_importance_sampling, pool);
}

float Prop::opacity(const Ray& ray, Sampler_filter filter, const Worker& worker) const {
	if (!has_masked_material()) {
		return intersect_p(ray, worker.node_stack()) ? 1.f : 0.f;
	}

	if (!visible_in_shadow()) {
		return 0.f;
	}

	if (shape_->is_complex() && !aabb_.intersect_p(ray)) {
		return 0.f;
	}

	entity::Composed_transformation temp;
	const auto& transformation = transformation_at(ray.time, temp);

	return shape_->opacity(transformation, ray, materials_, filter, worker);
}

float3 Prop::thin_absorption(const Ray& ray, Sampler_filter filter, const Worker& worker) const {
	if (!has_tinted_shadow()) {
		return float3(opacity(ray, filter, worker));
	}

	if (!visible_in_shadow()) {
		return float3(0.f);
	}

	if (shape_->is_complex() && !aabb_.intersect_p(ray)) {
		return float3(0.f);
	}

	entity::Composed_transformation temp;
	const auto& transformation = transformation_at(ray.time, temp);

	return shape_->thin_absorption(transformation, ray, materials_, filter, worker);
}

float Prop::area(uint32_t part) const {
	return parts_[part].area;
}

uint32_t Prop::light_id(uint32_t part) const {
	return parts_[part].light_id;
}

material::Material* Prop::material(uint32_t part) const {
	return materials_[part].get();
}

bool Prop::has_masked_material() const {
	return properties_.test(Properties::Masked_material);
}

bool Prop::has_tinted_shadow() const {
	return properties_.test(Properties::Tinted_shadow);
}

bool Prop::is_open() const {
	return properties_.test(Properties::Open);
}

void Prop::set_open(bool open) {
	properties_.set(Properties::Open, open);
}

size_t Prop::num_bytes() const {
	return sizeof(*this);
}

}
