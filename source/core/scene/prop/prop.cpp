#include "prop.hpp"
#include "base/math/aabb.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/quaternion.inl"
#include "base/math/transformation.inl"
#include "base/math/vector3.inl"
#include "scene/entity/composed_transformation.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.hpp"
#include "scene/shape/morphable_shape.hpp"
#include "scene/shape/shape.hpp"
#include "scene/shape/shape_intersection.hpp"

namespace scene::prop {

Prop::~Prop() noexcept {
    delete[] parts_;
    delete[] materials_;
}

void Prop::set_shape_and_materials(Shape* shape, Material* const* materials) noexcept {
    if (!shape_ || shape_->num_parts() != shape->num_parts()) {
        delete[] parts_;
        delete[] materials_;

        parts_     = new Part[shape->num_parts()];
        materials_ = new Material*[shape->num_parts()];
    }

    set_shape(shape);

    for (uint32_t i = 0, len = shape->num_parts(); i < len; ++i) {
        auto& p    = parts_[i];
        p.area     = 1.f;
        p.light_id = 0xFFFFFFFF;

        auto const m = materials[i];

        materials_[i] = m;

        if (m->is_masked()) {
            properties_.set(Property::Masked_material);
        }

        if (m->has_tinted_shadow()) {
            properties_.set(Property::Tinted_shadow);
        }
    }
}

void Prop::morph(thread::Pool& pool) noexcept {
    if (shape::Morphable_shape* morphable = shape_->morphable_shape(); morphable) {
        auto const& m = frames_[num_world_frames_].morphing;
        morphable->morph(m.targets[0], m.targets[1], m.weight, pool);
    }
}

bool Prop::intersect(Ray& ray, Node_stack& node_stack, shape::Intersection& intersection) const
    noexcept {
    if (!visible(ray.depth)) {
        return false;
    }

    if (shape_->is_complex() && !aabb_.intersect_p(ray)) {
        return false;
    }

    Transformation temp;
    auto const&    transformation = transformation_at(ray.time, temp);

    return shape_->intersect(ray, transformation, node_stack, intersection);
}

bool Prop::intersect_fast(Ray& ray, Node_stack& node_stack, shape::Intersection& intersection) const
    noexcept {
    if (!visible(ray.depth)) {
        return false;
    }

    if (shape_->is_complex() && !aabb_.intersect_p(ray)) {
        return false;
    }

    Transformation temp;
    auto const&    transformation = transformation_at(ray.time, temp);

    return shape_->intersect_fast(ray, transformation, node_stack, intersection);
}

bool Prop::intersect(Ray& ray, Node_stack& node_stack, shape::Normals& normals) const noexcept {
    //	if (!visible(ray.depth)) {
    //		return false;
    //	}

    if (!visible_in_shadow()) {
        return false;
    }

    if (shape_->is_complex() && !aabb_.intersect_p(ray)) {
        return false;
    }

    Transformation temp;
    auto const&    transformation = transformation_at(ray.time, temp);

    return shape_->intersect(ray, transformation, node_stack, normals);
}

bool Prop::intersect_p(Ray const& ray, Node_stack& node_stack) const noexcept {
    if (!visible_in_shadow()) {
        return false;
    }

    if (shape_->is_complex() && !aabb_.intersect_p(ray)) {
        return false;
    }

    Transformation temp;
    auto const&    transformation = transformation_at(ray.time, temp);

    return shape_->intersect_p(ray, transformation, node_stack);
}

// bool Prop::intersect_p(FVector ray_origin, FVector ray_direction,
//					   FVector ray_inv_direction, FVector ray_min_t, FVector
// ray_max_t, 					   float ray_time, shape::Node_stack& node_stack)
// const { if (!visible_in_shadow()) { 		return false;
//	}

//	if (shape_->is_complex()
//	&& !aabb_.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
//		return false;
//	}

//	entity::Composed_transformation temp;
//	auto const& transformation = transformation_at(ray_time, temp);

//	return shape_->intersect_p(ray_origin, ray_direction, ray_min_t, ray_max_t,
//							   transformation, node_stack);
//}

shape::Shape const* Prop::shape() const noexcept {
    return shape_;
}

shape::Shape* Prop::shape() noexcept {
    return shape_;
}

AABB const& Prop::aabb() const noexcept {
    return aabb_;
}

void Prop::set_shape(Shape* shape) noexcept {
    shape_ = shape;

    properties_.clear();
    properties_.set(Property::Visible_in_camera);
    properties_.set(Property::Visible_in_reflection);
    properties_.set(Property::Visible_in_shadow);
}

bool Prop::visible(uint32_t ray_depth) const noexcept {
    if (0 == ray_depth) {
        if (!properties_.test(Property::Visible_in_camera)) {
            return false;
        }
    } else {
        if (!properties_.test(Property::Visible_in_reflection)) {
            return false;
        }
    }

    return true;
}

void Prop::on_set_transformation() noexcept {
    if (1 == num_world_frames_) {
        aabb_ = shape_->transformed_aabb(world_transformation_.object_to_world,
                                         frames_[0].transformation);
    } else {
        static uint32_t constexpr num_steps = 4;

        static float constexpr interval = 1.f / static_cast<float>(num_steps);

        AABB aabb = shape_->transformed_aabb(frames_[0].transformation);

        for (uint32_t i = 0, len = num_world_frames_ - 1; i < len; ++i) {
            auto const& a = frames_[i].transformation;
            auto const& b = frames_[i + 1].transformation;

            float t = interval;
            for (uint32_t j = num_steps - 1; j > 0; --j, t += interval) {
                math::Transformation const interpolated = lerp(a, b, t);

                aabb.merge_assign(shape_->transformed_aabb(interpolated));
            }

            aabb_ = aabb.merge(shape_->transformed_aabb(b));
        }
    }
}

void Prop::set_parameters(json::Value const& /*parameters*/) noexcept {}

void Prop::prepare_sampling(uint32_t part, uint32_t light_id, uint64_t time,
                            bool material_importance_sampling, thread::Pool& pool) noexcept {
    shape_->prepare_sampling(part);

    Transformation temp;
    auto const&    transformation = transformation_at(time, temp);

    float const area  = shape_->area(part, transformation.scale);
    parts_[part].area = area;

    parts_[part].light_id = light_id;

    materials_[part]->prepare_sampling(*shape_, part, time, transformation, area,
                                       material_importance_sampling, pool);
}

void Prop::prepare_sampling_volume(uint32_t part, uint32_t light_id, uint64_t time,
                                   bool material_importance_sampling, thread::Pool& pool) noexcept {
    shape_->prepare_sampling(part);

    Transformation temp;
    auto const&    transformation = transformation_at(time, temp);

    float const volume  = shape_->volume(part, transformation.scale);
    parts_[part].volume = volume;

    parts_[part].light_id = light_id;

    materials_[part]->prepare_sampling(*shape_, part, time, transformation, volume,
                                       material_importance_sampling, pool);
}

float Prop::opacity(Ray const& ray, Filter filter, Worker const& worker) const noexcept {
    if (!has_masked_material()) {
        return intersect_p(ray, worker.node_stack()) ? 1.f : 0.f;
    }

    if (!visible_in_shadow()) {
        return 0.f;
    }

    if (shape_->is_complex() && !aabb_.intersect_p(ray)) {
        return 0.f;
    }

    Transformation temp;
    auto const&    transformation = transformation_at(ray.time, temp);

    return shape_->opacity(ray, transformation, materials_, filter, worker);
}

shape::Visibility Prop::thin_absorption(Ray const& ray, Filter filter, Worker const& worker,
                                        float3& ta) const noexcept {
    if (!has_tinted_shadow()) {
        float const o = opacity(ray, filter, worker);

        ta = float3(1.f - o);
        return 0.f == o ? Visibility::Complete : Visibility::None;
    }

    if (!visible_in_shadow()) {
        return Visibility::Complete;
    }

    if (shape_->is_complex() && !aabb_.intersect_p(ray)) {
        return Visibility::Complete;
    }

    Transformation temp;
    auto const&    transformation = transformation_at(ray.time, temp);

    return shape_->thin_absorption(ray, transformation, materials_, filter, worker, ta);
}

float Prop::area(uint32_t part) const noexcept {
    return parts_[part].area;
}

float Prop::volume(uint32_t part) const noexcept {
    return parts_[part].volume;
}

uint32_t Prop::light_id(uint32_t part) const noexcept {
    return parts_[part].light_id;
}

material::Material const* Prop::material(uint32_t part) const noexcept {
    return materials_[part];
}

bool Prop::has_masked_material() const noexcept {
    return properties_.test(Property::Masked_material);
}

bool Prop::has_caustic_material() const noexcept {
    for (uint32_t i = 0, len = shape_->num_parts(); i < len; ++i) {
        if (materials_[i]->is_caustic()) {
            return true;
        }
    }

    return false;
}

bool Prop::has_tinted_shadow() const noexcept {
    return properties_.test(Property::Tinted_shadow);
}

bool Prop::has_no_surface() const noexcept {
    return 1 == shape_->num_parts() && 1.f == materials_[0]->ior();
}

size_t Prop::num_bytes() const noexcept {
    return sizeof(*this);
}

}  // namespace scene::prop
