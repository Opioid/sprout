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
