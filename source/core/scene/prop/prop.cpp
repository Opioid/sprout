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
#include "scene/shape/morphable_shape.hpp"
#include "scene/shape/shape.hpp"
#include "scene/shape/shape_intersection.hpp"

namespace scene::prop {

Prop_material::Prop_material() noexcept : materials(nullptr), parts(nullptr) {}

Prop_material::Prop_material(Prop_material&& other) noexcept
    : materials(other.materials), parts(other.parts) {
    other.materials = nullptr;
    other.parts     = nullptr;
}

Prop_material::~Prop_material() noexcept {
    memory::free_aligned(parts);
    memory::free_aligned(materials);
}

Prop_frames::Prop_frames() noexcept : num_world_frames(0), num_local_frames(0), frames(nullptr) {}

Prop_frames::Prop_frames(Prop_frames&& other) noexcept
    : num_world_frames(other.num_world_frames),
      num_local_frames(other.num_local_frames),
      frames(other.frames) {
    other.frames = nullptr;
}

Prop_frames::~Prop_frames() noexcept {
    memory::free_aligned(frames);
}

Prop::Prop() noexcept = default;

Prop::~Prop() noexcept {}

void Prop::allocate_frames(uint32_t self, uint32_t num_world_frames, Scene const& scene) noexcept {
    Shape const* shape = scene.prop_shape(self);

    bool const is_animated = num_world_frames > 1;

    properties_.set(Property::Test_AABB,
                    shape->is_finite() && (shape->is_complex() || is_animated));

    properties_.set(Property::Static, !is_animated);
}

uint32_t Prop::shape() const noexcept {
    return shape_;
}

bool Prop::has_no_parent() const noexcept {
    return properties_.no(Property::Has_parent);
}

bool Prop::visible_in_camera() const noexcept {
    return properties_.is(Property::Visible_in_camera);
}

bool Prop::visible_in_reflection() const noexcept {
    return properties_.is(Property::Visible_in_reflection);
}

bool Prop::visible_in_shadow() const noexcept {
    return properties_.is(Property::Visible_in_shadow);
}

void Prop::set_visible_in_shadow(bool value) noexcept {
    properties_.set(Property::Visible_in_shadow, value);
}

void Prop::set_visibility(bool in_camera, bool in_reflection, bool in_shadow) noexcept {
    properties_.set(Property::Visible_in_camera, in_camera);
    properties_.set(Property::Visible_in_reflection, in_reflection);
    properties_.set(Property::Visible_in_shadow, in_shadow);
}

void Prop::set_has_parent() noexcept {
    properties_.set(Property::Has_parent);
}

void Prop::configure(Shape_ptr shape, Material_ptr const* materials) noexcept {
    set_shape(shape.id);

    for (uint32_t i = 0, len = shape.ptr->num_materials(); i < len; ++i) {
        auto const m = materials[i];

        if (m.ptr->is_masked()) {
            properties_.set(Property::Masked_material);
        }

        if (m.ptr->has_tinted_shadow()) {
            properties_.set(Property::Tinted_shadow);
        }
    }
}

bool Prop::intersect(uint32_t self, Ray& ray, Worker const& worker,
                     shape::Intersection& intersection) const noexcept {
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

    return scene.prop_shape(self)->intersect(ray, transformation, worker.node_stack(),
                                             intersection);
}

bool Prop::intersect_fast(uint32_t self, Ray& ray, Worker const& worker,
                          shape::Intersection& intersection) const noexcept {
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

    return scene.prop_shape(self)->intersect_fast(ray, transformation, worker.node_stack(),
                                                  intersection);
}

bool Prop::intersect(uint32_t self, Ray& ray, Worker const& worker, shape::Normals& normals) const
    noexcept {
    //	if (!visible(ray.depth)) {
    //		return false;
    //	}

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

bool Prop::intersect_p(uint32_t self, Ray const& ray, Worker const& worker) const noexcept {
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

void Prop::set_shape(uint32_t shape) noexcept {
    shape_ = shape;

    properties_.clear();
    properties_.set(Property::Visible_in_camera);
    properties_.set(Property::Visible_in_reflection);
    properties_.set(Property::Visible_in_shadow);
}

bool Prop::visible(uint32_t ray_depth) const noexcept {
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

float Prop::opacity(uint32_t self, Ray const& ray, Filter filter, Worker const& worker) const
    noexcept {
    if (!has_masked_material()) {
        return intersect_p(self, ray, worker) ? 1.f : 0.f;
    }

    if (!visible_in_shadow()) {
        return 0.f;
    }

    auto const& scene = worker.scene();

    if (properties_.is(Property::Test_AABB) && !scene.prop_aabb_intersect_p(self, ray)) {
        return 0.f;
    }

    Transformation temp;
    auto const&    transformation = scene.prop_transformation_at(self, ray.time, temp);

    return scene.prop_shape(self)->opacity(ray, transformation, self, filter, worker);
}

bool Prop::thin_absorption(uint32_t self, Ray const& ray, Filter filter, Worker const& worker,
                           float3& ta) const noexcept {
    if (!has_tinted_shadow()) {
        float const o = opacity(self, ray, filter, worker);

        ta = float3(1.f - o);
        return 0.f == o;
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

bool Prop::has_masked_material() const noexcept {
    return properties_.is(Property::Masked_material);
}

bool Prop::has_tinted_shadow() const noexcept {
    return properties_.is(Property::Tinted_shadow);
}

size_t Prop::num_bytes() const noexcept {
    return sizeof(*this);
}

}  // namespace scene::prop
