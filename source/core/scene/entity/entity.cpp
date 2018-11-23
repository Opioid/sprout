#include "entity.hpp"
#include "base/math/transformation.inl"
#include "composed_transformation.inl"
#include "scene/scene_constants.hpp"

namespace scene::entity {

Entity::~Entity() noexcept {
    delete[] local_frames_;
    delete[] world_frames_;
}

void Entity::allocate_frames(uint32_t num_frames) noexcept {
    num_world_frames_ = num_frames;

    world_frames_ = new Keyframe[num_frames];

    num_local_frames_ = num_frames;

    local_frames_ = new Keyframe[num_frames];
}

void Entity::allocate_local_frame() noexcept {
    num_local_frames_ = 1;

    local_frames_ = new Keyframe[1];
}

void Entity::propagate_frame_allocation() noexcept {
    if (!parent_) {
        if (0 == num_world_frames_) {
            num_world_frames_ = num_local_frames_;

            world_frames_ = new Keyframe[num_world_frames_];
        }
    }

    if (child_) {
        child_->inherit_frame_allocation(num_local_frames_);
    }
}

math::Transformation const& Entity::local_frame_0() const noexcept {
    return local_frames_[0].transformation;
}

Composed_transformation const& Entity::transformation_at(uint64_t        time,
                                                         Transformation& transformation) const
    noexcept {
    if (1 == num_world_frames_) {
        return world_transformation_;
    }

    for (uint32_t i = 0, len = num_world_frames_ - 1; i < len; ++i) {
        auto const& a = world_frames_[i];
        auto const& b = world_frames_[i + 1];

        if (time >= a.time && time < b.time) {
            uint64_t const range = b.time - a.time;
            uint64_t const delta = time - a.time;

            float const t = static_cast<float>(delta) / static_cast<float>(range);

            transformation.set(math::lerp(a.transformation, b.transformation, t));

            break;
        }
    }

    return transformation;
}

void Entity::set_transformation(math::Transformation const& t) noexcept {
    local_frames_[0].transformation = t;
    local_frames_[0].time           = scene::Static_time;
}

void Entity::set_frames(Keyframe const* frames, uint32_t num_frames) noexcept {
    for (uint32_t i = 0; i < num_frames; ++i) {
        local_frames_[i] = frames[i];
    }
}

void Entity::calculate_world_transformation() noexcept {
    if (!parent_) {
        for (uint32_t i = 0, len = num_world_frames_; i < len; ++i) {
            world_frames_[i] = local_frames_[i];
        }

        propagate_transformation();
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

void Entity::attach(Entity* node) noexcept {
    node->detach();

    node->parent_ = this;

    if (!child_) {
        child_ = node;
    } else {
        child_->add_sibling(node);
    }
}

void Entity::detach() noexcept {
    if (parent_) {
        parent_->detach(this);
    }
}

Entity const* Entity::parent() const noexcept {
    return parent_;
}

void Entity::propagate_transformation() noexcept {
    if (1 == num_world_frames_) {
        world_transformation_.set(world_frames_[0].transformation);
    }

    on_set_transformation();

    if (child_) {
        child_->inherit_transformation(world_frames_, num_world_frames_);
    }
}

void Entity::inherit_transformation(Keyframe const* frames, uint32_t num_frames) noexcept {
    if (next_) {
        next_->inherit_transformation(frames, num_frames);
    }

    for (uint32_t i = 0, len = num_world_frames_; i < len; ++i) {
        uint32_t const lf = num_local_frames_ > 1 ? i : 0;
        uint32_t const of = num_frames > 1 ? i : 0;
        local_frames_[lf].transform(world_frames_[i], frames[of]);
    }

    propagate_transformation();
}

void Entity::inherit_frame_allocation(uint32_t num_frames) noexcept {
    if (next_) {
        next_->inherit_frame_allocation(num_frames);
    }

    if (0 == num_world_frames_) {
        num_world_frames_ = num_frames;

        world_frames_ = new Keyframe[num_world_frames_];
    }

    propagate_frame_allocation();
}

void Entity::add_sibling(Entity* node) noexcept {
    if (!next_) {
        next_ = node;
    } else {
        next_->add_sibling(node);
    }
}

void Entity::detach(Entity* node) noexcept {
    // we can assume this to be true because of detach()
    // assert(node->parent_ == this);

    node->parent_ = nullptr;

    if (child_ == node) {
        child_      = node->next_;
        node->next_ = nullptr;
    } else {
        child_->remove_sibling(node);
    }
}

void Entity::remove_sibling(Entity* node) noexcept {
    if (next_ == node) {
        next_       = node->next_;
        node->next_ = nullptr;
    } else {
        next_->remove_sibling(node);
    }
}

}  // namespace scene::entity
