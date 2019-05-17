#include "entity.hpp"
#include "base/math/transformation.inl"
#include "base/memory/align.hpp"
#include "composed_transformation.inl"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"

namespace scene::entity {

Entity::Entity() noexcept = default;

Entity::~Entity() noexcept {
    memory::free_aligned(frames_);
}

void Entity::allocate_frames(uint32_t num_world_frames, uint32_t num_local_frames) noexcept {
    num_world_frames_ = num_world_frames;
    num_local_frames_ = num_local_frames;

    frames_ = memory::allocate_aligned<Keyframe>(num_world_frames + num_local_frames);
}

math::Transformation const& Entity::local_frame_0() const noexcept {
    return frames_[num_world_frames_].transformation;
}

Composed_transformation const& Entity::transformation_at(uint64_t        time,
                                                         Transformation& transformation) const
    noexcept {
    if (1 == num_world_frames_) {
        return world_transformation_;
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

void Entity::set_transformation(math::Transformation const& t) noexcept {
    Keyframe& local_frame = frames_[num_world_frames_];

    local_frame.transformation = t;
    local_frame.time           = scene::Static_time;
}

void Entity::set_frames(Keyframe const* frames, uint32_t num_frames) noexcept {
    Keyframe* local_frames = &frames_[num_world_frames_];
    for (uint32_t i = 0; i < num_frames; ++i) {
        local_frames[i] = frames[i];
    }
}

void Entity::calculate_world_transformation(Scene const& scene) noexcept {
    if (Null == parent_) {
        for (uint32_t i = 0, len = num_world_frames_; i < len; ++i) {
            frames_[i] = frames_[len + i];
        }

        propagate_transformation(scene);
    }
}

bool Entity::visible_in_camera() const noexcept {
    return properties_.test(Property::Visible_in_camera);
}

bool Entity::visible_in_reflection() const noexcept {
    return properties_.test(Property::Visible_in_reflection);
}

bool Entity::visible_in_shadow() const noexcept {
    return properties_.test(Property::Visible_in_shadow);
}

void Entity::set_visible_in_shadow(bool value) noexcept {
    properties_.set(Property::Visible_in_shadow, value);
}

void Entity::set_visibility(bool in_camera, bool in_reflection, bool in_shadow) noexcept {
    properties_.set(Property::Visible_in_camera, in_camera);
    properties_.set(Property::Visible_in_reflection, in_reflection);
    properties_.set(Property::Visible_in_shadow, in_shadow);
}

void Entity::attach(uint32_t self, uint32_t node, Scene const& scene) noexcept {
    Entity* n = scene.entity(node);

    n->detach_self(self, scene);

    n->parent_ = self;

    if (0 == n->num_local_frames_) {
        // This is the case if n has no animation attached to it directly
        n->allocate_frames(num_world_frames_, 1);
    }

    if (Null == child_) {
        child_ = node;
    } else {
        scene.entity(child_)->add_sibling(node, scene);
    }
}

void Entity::detach_self(uint32_t self, Scene const& scene) noexcept {
    if (Null != parent_) {
        scene.entity(parent_)->detach(self, scene);
    }
}

void Entity::propagate_transformation(Scene const& scene) noexcept {
    if (1 == num_world_frames_) {
        world_transformation_.set(frames_[0].transformation);
    }

    on_set_transformation();

    if (Null != child_) {
        scene.entity(child_)->inherit_transformation(frames_, num_world_frames_, scene);
    }
}

void Entity::inherit_transformation(Keyframe const* frames, uint32_t num_frames,
                                    Scene const& scene) noexcept {
    if (Null != next_) {
        scene.entity(next_)->inherit_transformation(frames, num_frames, scene);
    }

    for (uint32_t i = 0, len = num_world_frames_; i < len; ++i) {
        uint32_t const lf = num_local_frames_ > 1 ? i : 0;
        uint32_t const of = num_frames > 1 ? i : 0;
        frames_[len + lf].transform(frames_[i], frames[of]);
    }

    propagate_transformation(scene);
}

void Entity::add_sibling(uint32_t node, Scene const& scene) noexcept {
    if (Null == next_) {
        next_ = node;
    } else {
        scene.entity(next_)->add_sibling(node, scene);
    }
}

void Entity::detach(uint32_t node, Scene const& scene) noexcept {
    // we can assume this to be true because of detach()
    // assert(node->parent_ == this);

    Entity* n = scene.entity(node);

    n->parent_ = Null;

    if (child_ == node) {
        child_   = n->next_;
        n->next_ = Null;
    } else {
        scene.entity(child_)->remove_sibling(node, scene);
    }
}

void Entity::remove_sibling(uint32_t node, Scene const& scene) noexcept {
    Entity* n = scene.entity(node);

    if (next_ == node) {
        next_    = n->next_;
        n->next_ = Null;
    } else {
        n->remove_sibling(node, scene);
    }
}

}  // namespace scene::entity
