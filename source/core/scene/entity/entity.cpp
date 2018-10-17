#include "entity.hpp"
#include "base/math/transformation.inl"
#include "composed_transformation.inl"

namespace scene::entity {

Entity::~Entity() noexcept {
    delete [] local_frames_;
    delete [] world_frames_;
}

void Entity::allocate_frames(uint32_t num_frames) noexcept {
    num_world_frames_ = num_frames;

    world_frames_ = new math::Transformation[num_frames];

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

            world_frames_ = new math::Transformation[num_world_frames_];
        }
    }

        if (child_) {
            child_->inherit_frame_allocation(num_local_frames_);
        }

}

math::Transformation const& Entity::local_frame_0() const noexcept {
    return local_frames_[0].transformation;
}

Composed_transformation const& Entity::transformation_at(uint64_t           time,
                                                         Transformation& transformation) const
    noexcept {
    if (properties_.test_not(Property::Animated)) {
        return world_transformation_;
    }

 //   transformation.set(math::lerp(world_frame_a_, world_frame_b_, tick_delta));

        for (uint32_t i = 0, len = num_world_frames_ - 1; i < len; ++i) {
            uint64_t const a_time = local_frames_[i].time_i;
            uint64_t const b_time = local_frames_[i + 1].time_i;

            if (time >= a_time && time < b_time) {
                uint64_t const range = b_time - a_time;
                uint64_t const delta = time - a_time;

                float const t = static_cast<float>(delta) / static_cast<float>(range);

                auto const& a = world_frames_[i];
                auto const& b = world_frames_[i + 1];

                transformation.set(math::lerp(a, b, t));

                break;
            }
        }


    return transformation;
}

void Entity::set_transformation(math::Transformation const& t) noexcept {
    world_transformation_.set(t);

    local_frames_[0].transformation = t;

    properties_.unset(Property::Animated);

    propagate_transformation();

    on_set_transformation();
}

void Entity::tick(Keyframe const& frame) noexcept {
 //   local_frame_0_ = local_frame_b_;
 //   local_frame_b_ = frame;

    // In the current implementation
    // "animation" means "transformation changes during simulation frame"
 //   bool const changed = local_frame_0_.transformation != local_frame_b_.transformation;

    properties_.set(Property::Animated, true);
}

void Entity::set_frames(Keyframe const* frames, uint32_t num_frames) noexcept {
    // num_local_frames_ = num_frames;

    for (uint32_t i = 0; i < num_frames; ++i) {
        local_frames_[i] = frames[i];
    }

    properties_.set(Property::Animated, true);
}

void Entity::calculate_world_transformation() noexcept {
    if (!parent_) {
        for (uint32_t i = 0, len = num_world_frames_; i < len; ++i) {
            world_frames_[i] = local_frames_[i].transformation;
        }

        propagate_transformation();

        on_set_transformation();
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

void Entity::set_visibility(bool in_camera, bool in_reflection, bool in_shadow,
                            bool propagate) noexcept {
    properties_.set(Property::Visible_in_camera, in_camera);
    properties_.set(Property::Visible_in_reflection, in_reflection);
    properties_.set(Property::Visible_in_shadow, in_shadow);

    if (next_ && propagate) {
        next_->set_visibility(in_camera, in_reflection, in_shadow, true);
    }

    if (child_ && properties_.test(Property::Propagate_visibility)) {
        child_->set_visibility(in_camera, in_reflection, in_shadow, true);
    }
}

void Entity::set_propagate_visibility(bool enable) noexcept {
    properties_.set(Property::Propagate_visibility, enable);
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

void Entity::propagate_transformation() const noexcept {
    if (child_) {
        child_->inherit_transformation(world_frames_, num_world_frames_,
                                       properties_.test(Property::Animated));
    }
}

void Entity::inherit_transformation(math::Transformation const* frames, uint32_t num_frames,
                                    bool animated) noexcept {
    if (next_) {
        next_->inherit_transformation(frames, num_frames, animated);
    }

    if (animated) {
        properties_.set(Property::Animated);
    }

//    local_frame_0_.transform(world_frame_a_, a);

//    local_frame_b_.transform(world_frame_b_, b);

//    if (properties_.test_not(Property::Animated)) {
//        world_transformation_.set(world_frame_a_);
//    }

    on_set_transformation();

    propagate_transformation();
}

void Entity::inherit_frame_allocation(uint32_t num_frames) noexcept {
    if (next_) {
        next_->inherit_frame_allocation(num_frames);
    }

    if (0 == num_world_frames_) {
        num_world_frames_ = num_frames;

        world_frames_ = new math::Transformation[num_world_frames_];
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
