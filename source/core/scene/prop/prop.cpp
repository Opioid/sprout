#include "prop.hpp"
#include "base/math/aabb.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/quaternion.inl"
#include "base/math/transformation.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "resource/resource.hpp"
#include "scene/animation/animation.hpp"
#include "scene/entity/composed_transformation.inl"
#include "scene/scene.inl"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.inl"
#include "scene/shape/morphable.hpp"
#include "scene/shape/shape.inl"
#include "scene/shape/shape_intersection.hpp"

namespace scene::prop {

using Transformation = entity::Composed_transformation;

Prop::Prop() = default;

Prop::~Prop() = default;

void Prop::set_visible_in_shadow(bool value) {
    properties_.set(Property::Visible_in_shadow, value);
}

void Prop::set_visibility(bool in_camera, bool in_reflection, bool in_shadow) {
    properties_.set(Property::Visible_in_camera, in_camera);
    properties_.set(Property::Visible_in_reflection, in_reflection);
    properties_.set(Property::Visible_in_shadow, in_shadow);
}

void Prop::configure(Shape_ptr shape, Material_ptr const* materials) {
    shape_ = shape.id;

    properties_.clear();

    properties_.set(Property::Visible_in_camera);
    properties_.set(Property::Visible_in_reflection);
    properties_.set(Property::Visible_in_shadow);

    properties_.set(Property::Test_AABB, shape.ptr->is_finite() && shape.ptr->is_complex());

    properties_.set(Property::Static, true);

    for (uint32_t i = 0, len = shape.ptr->num_materials(); i < len; ++i) {
        auto const m = materials[i].ptr;

        if (m->is_masked()) {
            properties_.set(Property::Masked_material);
        }

        if (m->has_tinted_shadow()) {
            properties_.set(Property::Tinted_shadow);
        }
    }
}

void Prop::configure_animated(uint32_t self, bool local_animation, Scene const& scene) {
    Shape const* shape = scene.prop_shape(self);

    properties_.set(Property::Test_AABB, shape->is_finite());
    properties_.set(Property::Static, false);
    properties_.set(Property::Local_animation, local_animation);
}

bool Prop::intersect(uint32_t self, Ray& ray, Worker& worker, shape::Intersection& isec) const {
    if (!visible(ray.depth)) {
        return false;
    }

    bool const is_static = properties_.is(Property::Static);

    auto const& scene = worker.scene();

    if (properties_.is(Property::Test_AABB) && !scene.prop_aabb_intersect_p(self, ray)) {
        return false;
    }

    Transformation temp;
    auto const&    transformation = scene.prop_transformation_at(self, ray.time, is_static, temp);

    return scene.prop_shape(self)->intersect(ray, transformation, worker.node_stack(), isec);
}

bool Prop::intersect_nsf(uint32_t self, Ray& ray, Worker& worker, shape::Intersection& isec) const {
    if (!visible(ray.depth)) {
        return false;
    }

    bool const is_static = properties_.is(Property::Static);

    auto const& scene = worker.scene();

    if (properties_.is(Property::Test_AABB) && !scene.prop_aabb_intersect_p(self, ray)) {
        return false;
    }

    Transformation temp;
    auto const&    transformation = scene.prop_transformation_at(self, ray.time, is_static, temp);

    return scene.prop_shape(self)->intersect_nsf(ray, transformation, worker.node_stack(), isec);
}

bool Prop::intersect(uint32_t self, Ray& ray, Worker& worker, shape::Normals& normals) const {
    if (!visible_in_shadow()) {
        return false;
    }

    bool const is_static = properties_.is(Property::Static);

    auto const& scene = worker.scene();

    if (properties_.is(Property::Test_AABB) && !scene.prop_aabb_intersect_p(self, ray)) {
        return false;
    }

    Transformation temp;
    auto const&    transformation = scene.prop_transformation_at(self, ray.time, is_static, temp);

    return scene.prop_shape(self)->intersect(ray, transformation, worker.node_stack(), normals);
}

bool Prop::intersect_p(uint32_t self, Ray const& ray, Worker& worker) const {
    if (!visible_in_shadow()) {
        return false;
    }

    bool const is_static = properties_.is(Property::Static);

    auto const& scene = worker.scene();

    if (properties_.is(Property::Test_AABB) && !scene.prop_aabb_intersect_p(self, ray)) {
        return false;
    }

    Transformation temp;
    auto const&    transformation = scene.prop_transformation_at(self, ray.time, is_static, temp);

    return scene.prop_shape(self)->intersect_p(ray, transformation, worker.node_stack());
}

bool Prop::visible(uint32_t ray_depth) const {
    if (0 == ray_depth) {
        if (!properties_.is(Property::Visible_in_camera)) {
            return false;
        }
    } else {
        if (!properties_.is(Property::Visible_in_reflection)) {
            return false;
        }
    }

    return true;
}

float Prop::visibility(uint32_t self, Ray const& ray, Filter filter, Worker& worker) const {
    if (!has_masked_material()) {
        return intersect_p(self, ray, worker) ? 0.f : 1.f;
    }

    if (!visible_in_shadow()) {
        return 1.f;
    }

    auto const& scene = worker.scene();

    if (properties_.is(Property::Test_AABB) && !scene.prop_aabb_intersect_p(self, ray)) {
        return 1.f;
    }

    Transformation temp;
    auto const&    transformation = scene.prop_transformation_at(self, ray.time, temp);

    return scene.prop_shape(self)->visibility(ray, transformation, self, filter, worker);
}

bool Prop::thin_absorption(uint32_t self, Ray const& ray, Filter filter, Worker& worker,
                           float3& ta) const {
    if (!has_tinted_shadow()) {
        float const v = visibility(self, ray, filter, worker);

        ta = float3(v);
        return v > 0.f;
    }

    if (!visible_in_shadow()) {
        ta = float3(1.f);
        return true;
    }

    auto const& scene = worker.scene();

    if (properties_.is(Property::Test_AABB) && !scene.prop_aabb_intersect_p(self, ray)) {
        ta = float3(1.f);
        return true;
    }

    Transformation temp;
    auto const&    transformation = scene.prop_transformation_at(self, ray.time, temp);

    return scene.prop_shape(self)->thin_absorption(ray, transformation, self, filter, worker, ta);
}

bool Prop::has_masked_material() const {
    return properties_.is(Property::Masked_material);
}

bool Prop::has_tinted_shadow() const {
    return properties_.is(Property::Tinted_shadow);
}

}  // namespace scene::prop
