#include "entity.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/transformation.inl"

namespace scene { namespace entity {

Entity::Entity() : parent_(nullptr), next_(nullptr), child_(nullptr) {}

Entity::~Entity() {}

const math::Transformation& Entity::local_frame_a() const {
	return local_frame_a_.transformation;
}

const Composed_transformation& Entity::transformation_at(
		float tick_delta, Composed_transformation& transformation) const {
	if (!properties_.test(Properties::Animated)) {
		return world_transformation_;
	}

	transformation.set(math::lerp(world_frame_a_, world_frame_b_, tick_delta));

	return transformation;
}

void Entity::set_transformation(const math::Transformation& t) {
	world_transformation_.set(t);

	local_frame_a_.transformation = t;
	local_frame_b_.transformation = t;

	world_frame_a_ = t;
	world_frame_b_ = t;

	properties_.unset(Properties::Animated);

	propagate_transformation();

	on_set_transformation();
}

void Entity::tick(const Keyframe& frame) {
	local_frame_a_ = local_frame_b_;
	local_frame_b_ = frame;

	// In the current implementation
	// "animation" means "transformation changes during simulation frame"
	bool changed_transformation = local_frame_a_.transformation != local_frame_b_.transformation;

	properties_.set(Properties::Animated, changed_transformation);
}

void Entity::calculate_world_transformation() {
	if (!parent_) {
		world_frame_a_ = local_frame_a_.transformation;
		world_frame_b_ = local_frame_b_.transformation;

		propagate_transformation();

		on_set_transformation();
	}
}

bool Entity::visible_in_camera() const {
	return properties_.test(Properties::Visible_in_camera);
}

bool Entity::visible_in_reflection() const {
	return properties_.test(Properties::Visible_in_reflection);
}

bool Entity::visible_in_shadow() const {
	return properties_.test(Properties::Visible_in_shadow);
}

void Entity::set_visibility(bool in_camera, bool in_reflection, bool in_shadow) {
	properties_.set(Properties::Visible_in_camera,		in_camera);
	properties_.set(Properties::Visible_in_reflection,	in_reflection);
	properties_.set(Properties::Visible_in_shadow,		in_shadow);

	if (properties_.test(Properties::Propagate_visibility)) {
		if (next_) {
			next_->set_visibility(in_camera, in_reflection, in_shadow);
		}

		if (child_) {
			child_->set_visibility(in_camera, in_reflection, in_shadow);
		}
	}
}

void Entity::set_propagate_visibility(bool enable) {
	properties_.set(Properties::Propagate_visibility, enable);
}

void Entity::attach(Entity* node) {
	node->detach();

	node->parent_ = this;

	if (!child_) {
		child_ = node;
	} else {
		child_->add_sibling(node);
	}
}

void Entity::detach() {
	if (parent_) {
		parent_->detach(this);
	}
}

const Entity* Entity::parent() const {
	return parent_;
}

void Entity::propagate_transformation() const {
	if (child_) {
		child_->inherit_transformation(world_frame_a_, world_frame_b_,
									   properties_.test(Properties::Animated));
	}
}

void Entity::inherit_transformation(const math::Transformation& a,
									const math::Transformation& b,
									bool animated) {
	if (next_) {
		next_->inherit_transformation(a, b, animated);
	}

	if (animated) {
		properties_.set(Properties::Animated);
	}

	world_frame_a_.position = math::transform_point(local_frame_a_.transformation.position,
													float4x4(a));
	world_frame_a_.rotation = math::quaternion::mul(local_frame_a_.transformation.rotation,
													a.rotation);
	world_frame_a_.scale = local_frame_a_.transformation.scale;


	world_frame_b_.position = math::transform_point(local_frame_b_.transformation.position,
													float4x4(b));
	world_frame_b_.rotation = math::quaternion::mul(local_frame_b_.transformation.rotation,
													b.rotation);
	world_frame_b_.scale = local_frame_b_.transformation.scale;

	on_set_transformation();

	propagate_transformation();
}

void Entity::add_sibling(Entity* node) {
	if (!next_) {
		next_ = node;
	} else {
		next_->add_sibling(node);
	}
}

void Entity::detach(Entity* node) {
	// we can assume this to be true because of detach()
	// assert(node->parent_ == this);

	node->parent_ = nullptr;

	if (child_ == node) {
		child_ = node->next_;
		node->next_ = nullptr;
	} else {
		child_->remove_sibling(node);
	}
}

void Entity::remove_sibling(Entity* node) {
	if (next_ == node) {
		next_ = node->next_;
		node->next_ = nullptr;
	} else {
		next_->remove_sibling(node);
	}
}

}}
