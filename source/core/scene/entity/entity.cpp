#include "entity.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/transformation.inl"
#include "composed_transformation.inl"

namespace scene::entity {

Entity::~Entity() noexcept {}

math::Transformation const& Entity::local_frame_a() const noexcept {
    return local_frame_a_.transformation;
}

Composed_transformation const& Entity::transformation_at(float           tick_delta,
                                                         Transformation& transformation) const
    noexcept {
    if (!properties_.test(Property::Animated)) {
        return world_transformation_;
    }

    transformation.set(math::lerp(world_frame_a_, world_frame_b_, tick_delta));

    return transformation;
}

void Entity::set_transformation(math::Transformation const& t) noexcept {
    world_transformation_.set(t);

    local_frame_a_.transformation = t;
    local_frame_b_.transformation = t;

    world_frame_a_ = t;
    world_frame_b_ = t;

    properties_.unset(Property::Animated);

    propagate_transformation();

    on_set_transformation();
}

void Entity::tick(Keyframe const& frame) noexcept {
    local_frame_a_ = local_frame_b_;
    local_frame_b_ = frame;

    // In the current implementation
    // "animation" means "transformation changes during simulation frame"
    bool const changed = local_frame_a_.transformation != local_frame_b_.transformation;

    properties_.set(Property::Animated, changed);
}

void Entity::calculate_world_transformation() noexcept {
    if (!parent_) {
        world_frame_a_ = local_frame_a_.transformation;
        world_frame_b_ = local_frame_b_.transformation;

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
        child_->inherit_transformation(world_frame_a_, world_frame_b_,
                                       properties_.test(Property::Animated));
    }
}

void Entity::inherit_transformation(math::Transformation const& a, math::Transformation const& b,
                                    bool animated) noexcept {
    if (next_) {
        next_->inherit_transformation(a, b, animated);
    }

    if (animated) {
        properties_.set(Property::Animated);
    }

    world_frame_a_.position = math::transform_point(float4x4(a),
                                                    local_frame_a_.transformation.position);

    world_frame_a_.rotation = math::quaternion::mul(local_frame_a_.transformation.rotation,
                                                    a.rotation);

    world_frame_a_.scale = local_frame_a_.transformation.scale;

    world_frame_b_.position = math::transform_point(float4x4(b),
                                                    local_frame_b_.transformation.position);

    world_frame_b_.rotation = math::quaternion::mul(local_frame_b_.transformation.rotation,
                                                    b.rotation);

    world_frame_b_.scale = local_frame_b_.transformation.scale;

    if (!properties_.test(Property::Animated)) {
        world_transformation_.set(world_frame_a_);
    }

    on_set_transformation();

    propagate_transformation();
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
