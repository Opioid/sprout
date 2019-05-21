#include "prop.hpp"
#include "base/math/aabb.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/quaternion.inl"
#include "base/math/transformation.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "scene/animation/animation.hpp"
#include "scene/entity/composed_transformation.inl"
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.hpp"
#include "scene/shape/morphable_shape.hpp"
#include "scene/shape/shape.hpp"
#include "scene/shape/shape_intersection.hpp"

namespace scene::prop {

Prop::Prop() = default;

Prop::Prop(Prop&& other) noexcept
    : properties_(other.properties_),
      num_world_frames_(other.num_world_frames_),
      frames_(other.frames_),
      aabb_(other.aabb_),
      shape_(other.shape_),
      num_local_frames_(other.num_local_frames_),
      parent_(other.parent_),
      next_(other.next_),
      child_(other.child_) {
    other.frames_ = nullptr;
    other.shape_  = nullptr;
}

Prop::~Prop() noexcept {
    memory::free_aligned(frames_);
}

void Prop::allocate_frames(uint32_t num_world_frames, uint32_t num_local_frames) noexcept {
    num_world_frames_ = num_world_frames;
    num_local_frames_ = num_local_frames;

    frames_ = memory::allocate_aligned<Keyframe>(num_world_frames + num_local_frames);

    properties_.set(Property::Test_AABB,
                    shape_->is_finite() && (shape_->is_complex() || num_world_frames > 1));
}

math::Transformation const& Prop::local_frame_0() const noexcept {
    return frames_[num_world_frames_].transformation;
}

Prop::Transformation const& Prop::transformation_at(uint32_t self, uint64_t time,
                                                    Transformation& transformation,
                                                    Scene const&    scene) const noexcept {
    if (1 == num_world_frames_) {
        return scene.prop_world_transformation(self);
        //    return world_transformation_;
    }

    for (uint32_t i = 0, len = num_world_frames_ - 1; i < len; ++i) {
        auto const& a = frames_[i];
        auto const& b = frames_[i + 1];

        if (time >= a.time && time < b.time) {
            uint64_t const range = b.time - a.time;
            uint64_t const delta = time - a.time;

            float const t = static_cast<float>(delta) / static_cast<float>(range);

            transformation.set(lerp(a.transformation, b.transformation, t));

            break;
        }
    }

    return transformation;
}

void Prop::set_transformation(math::Transformation const& t) noexcept {
    Keyframe& local_frame = frames_[num_world_frames_];

    local_frame.transformation = t;
    local_frame.time           = scene::Static_time;
}

void Prop::set_frames(animation::Keyframe const* frames, uint32_t num_frames) noexcept {
    Keyframe* local_frames = &frames_[num_world_frames_];
    for (uint32_t i = 0; i < num_frames; ++i) {
        local_frames[i] = frames[i].k;
    }

    morphing_ = frames[0].m;
}

void Prop::calculate_world_transformation(uint32_t self, Scene& scene) noexcept {
    if (Null == parent_) {
        for (uint32_t i = 0, len = num_world_frames_; i < len; ++i) {
            frames_[i] = frames_[len + i];
        }

        propagate_transformation(self, scene);
    }
}

bool Prop::visible_in_camera() const noexcept {
    return properties_.test(Property::Visible_in_camera);
}

bool Prop::visible_in_reflection() const noexcept {
    return properties_.test(Property::Visible_in_reflection);
}

bool Prop::visible_in_shadow() const noexcept {
    return properties_.test(Property::Visible_in_shadow);
}

void Prop::set_visible_in_shadow(bool value) noexcept {
    properties_.set(Property::Visible_in_shadow, value);
}

void Prop::set_visibility(bool in_camera, bool in_reflection, bool in_shadow) noexcept {
    properties_.set(Property::Visible_in_camera, in_camera);
    properties_.set(Property::Visible_in_reflection, in_reflection);
    properties_.set(Property::Visible_in_shadow, in_shadow);
}

void Prop::attach(uint32_t self, uint32_t node, Scene& scene) noexcept {
    Prop* n = scene.prop(node);

    n->detach_self(self, scene);

    n->parent_ = self;

    if (0 == n->num_local_frames_) {
        // This is the case if n has no animation attached to it directly
        n->allocate_frames(num_world_frames_, 1);
    }

    if (Null == child_) {
        child_ = node;
    } else {
        scene.prop(child_)->add_sibling(node, scene);
    }
}

void Prop::detach_self(uint32_t self, Scene& scene) noexcept {
    if (Null != parent_) {
        scene.prop(parent_)->detach(self, scene);
    }
}

void Prop::set_shape_and_materials(Shape* shape, Material* const* materials) noexcept {
    set_shape(shape);

    for (uint32_t i = 0, len = shape->num_parts(); i < len; ++i) {
        auto const m = materials[i];

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
        auto const& m = morphing_;
        morphable->morph(m.targets[0], m.targets[1], m.weight, pool);
    }
}

bool Prop::intersect(uint32_t self, Ray& ray, Worker const& worker,
                     shape::Intersection& intersection) const noexcept {
    if (!visible(ray.depth)) {
        return false;
    }

    if (properties_.test(Property::Test_AABB) && !aabb_.intersect_p(ray)) {
        return false;
    }

    Transformation temp;
    auto const&    transformation = transformation_at(self, ray.time, temp, worker.scene());

    return shape_->intersect(ray, transformation, worker.node_stack(), intersection);
}

bool Prop::intersect_fast(uint32_t self, Ray& ray, Worker const& worker,
                          shape::Intersection& intersection) const noexcept {
    if (!visible(ray.depth)) {
        return false;
    }

    if (properties_.test(Property::Test_AABB) && !aabb_.intersect_p(ray)) {
        return false;
    }

    Transformation temp;
    auto const&    transformation = transformation_at(self, ray.time, temp, worker.scene());

    return shape_->intersect_fast(ray, transformation, worker.node_stack(), intersection);
}

bool Prop::intersect(uint32_t self, Ray& ray, Worker const& worker, shape::Normals& normals) const
    noexcept {
    //	if (!visible(ray.depth)) {
    //		return false;
    //	}

    if (!visible_in_shadow()) {
        return false;
    }

    if (properties_.test(Property::Test_AABB) && !aabb_.intersect_p(ray)) {
        return false;
    }

    Transformation temp;
    auto const&    transformation = transformation_at(self, ray.time, temp, worker.scene());

    return shape_->intersect(ray, transformation, worker.node_stack(), normals);
}

bool Prop::intersect_p(uint32_t self, Ray const& ray, Worker const& worker) const noexcept {
    if (!visible_in_shadow()) {
        return false;
    }

    if (properties_.test(Property::Test_AABB) && !aabb_.intersect_p(ray)) {
        return false;
    }

    Transformation temp;
    auto const&    transformation = transformation_at(self, ray.time, temp, worker.scene());

    return shape_->intersect_p(ray, transformation, worker.node_stack());
}

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

void Prop::on_set_transformation(uint32_t self, Scene const& scene) noexcept {
    if (1 == num_world_frames_) {
        auto const& t = scene.prop_world_transformation(self);

        aabb_ = shape_->transformed_aabb(t.object_to_world, frames_[0].transformation);
    } else {
        static uint32_t constexpr Num_steps = 4;

        static float constexpr Interval = 1.f / static_cast<float>(Num_steps);

        AABB aabb = shape_->transformed_aabb(frames_[0].transformation);

        for (uint32_t i = 0, len = num_world_frames_ - 1; i < len; ++i) {
            auto const& a = frames_[i].transformation;
            auto const& b = frames_[i + 1].transformation;

            float t = Interval;
            for (uint32_t j = Num_steps - 1; j > 0; --j, t += Interval) {
                math::Transformation const interpolated = lerp(a, b, t);

                aabb.merge_assign(shape_->transformed_aabb(interpolated));
            }

            aabb_ = aabb.merge(shape_->transformed_aabb(b));
        }
    }
}

float Prop::opacity(uint32_t self, Ray const& ray, Filter filter, Worker const& worker) const
    noexcept {
    if (!has_masked_material()) {
        return intersect_p(self, ray, worker) ? 1.f : 0.f;
    }

    if (!visible_in_shadow()) {
        return 0.f;
    }

    if (properties_.test(Property::Test_AABB) && !aabb_.intersect_p(ray)) {
        return 0.f;
    }

    Transformation temp;
    auto const&    transformation = transformation_at(self, ray.time, temp, worker.scene());

    return shape_->opacity(ray, transformation, worker.scene().prop_materials(self), filter,
                           worker);
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

    if (properties_.test(Property::Test_AABB) && !aabb_.intersect_p(ray)) {
        ta = float3(1.f);
        return true;
    }

    Transformation temp;
    auto const&    transformation = transformation_at(self, ray.time, temp, worker.scene());

    return shape_->thin_absorption(ray, transformation, worker.scene().prop_materials(self), filter,
                                   worker, ta);
}

bool Prop::has_masked_material() const noexcept {
    return properties_.test(Property::Masked_material);
}

bool Prop::has_tinted_shadow() const noexcept {
    return properties_.test(Property::Tinted_shadow);
}

size_t Prop::num_bytes() const noexcept {
    return sizeof(*this);
}

void Prop::propagate_transformation(uint32_t self, Scene& scene) noexcept {
    if (1 == num_world_frames_) {
        scene.prop_set_world_transformation(self, frames_[0].transformation);
    }

    on_set_transformation(self, scene);

    if (Null != child_) {
        scene.prop(child_)->inherit_transformation(self, frames_, num_world_frames_, scene);
    }
}

void Prop::inherit_transformation(uint32_t self, Keyframe const* frames, uint32_t num_frames,
                                  Scene& scene) noexcept {
    if (Null != next_) {
        scene.prop(next_)->inherit_transformation(self, frames, num_frames, scene);
    }

    for (uint32_t i = 0, len = num_world_frames_; i < len; ++i) {
        uint32_t const lf = num_local_frames_ > 1 ? i : 0;
        uint32_t const of = num_frames > 1 ? i : 0;
        frames_[len + lf].transform(frames_[i], frames[of]);
    }

    propagate_transformation(self, scene);
}

void Prop::add_sibling(uint32_t node, Scene& scene) noexcept {
    if (Null == next_) {
        next_ = node;
    } else {
        scene.prop(next_)->add_sibling(node, scene);
    }
}

void Prop::detach(uint32_t node, Scene& scene) noexcept {
    // we can assume this to be true because of detach()
    // assert(node->parent_ == this);

    Prop* n = scene.prop(node);

    n->parent_ = Null;

    if (child_ == node) {
        child_   = n->next_;
        n->next_ = Null;
    } else {
        scene.prop(child_)->remove_sibling(node, scene);
    }
}

void Prop::remove_sibling(uint32_t node, Scene& scene) noexcept {
    Prop* n = scene.prop(node);

    if (next_ == node) {
        next_    = n->next_;
        n->next_ = Null;
    } else {
        n->remove_sibling(node, scene);
    }
}

}  // namespace scene::prop
