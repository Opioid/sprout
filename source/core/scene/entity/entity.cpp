#include "entity.hpp"
#include "base/math/matrix.inl"
#include "base/math/transformation.inl"

namespace scene { namespace entity {

Entity::Entity() : parent_(nullptr), next_(nullptr), child_(nullptr) {}

Entity::~Entity() {}

bool Entity::transformation_at(float tick_delta, Composed_transformation& transformation) const {
	if (!animated_) {
		transformation = world_transformation_;
	} else {
		transformation.set(math::lerp(world_frame_a_, world_frame_b_, tick_delta));
	}

	return animated_;
}

void Entity::set_transformation(const math::transformation& t, thread::Pool& pool) {
	world_transformation_.set(t);

	local_frame_a_.transformation = t;
	local_frame_b_.transformation = t;

	world_frame_a_ = t;
	world_frame_b_ = t;

	animated_ = false;

	propagate_transformation(pool);

	on_set_transformation(pool);
}

void Entity::set_beginning(const Keyframe& frame) {
	local_frame_b_ = frame;
}

void Entity::tick(const Keyframe& frame) {
	local_frame_a_ = local_frame_b_;
	local_frame_b_ = frame;

	animated_ = true;
}

void Entity::calculate_world_transformation(thread::Pool& pool) {
	if (!parent_) {
		world_frame_a_ = local_frame_a_.transformation;
		world_frame_b_ = local_frame_b_.transformation;

		propagate_transformation(pool);

		on_set_transformation(pool);
	}
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

void Entity::propagate_transformation(thread::Pool& pool) const {
	if (child_) {
		child_->inherit_transformation(world_frame_a_, world_frame_b_, pool);
	}
}

void Entity::inherit_transformation(const math::transformation& a, const math::transformation& b, thread::Pool& pool) {
	if (next_) {
		next_->inherit_transformation(a, b, pool);
	}

	animated_ = true;

	math::float4x4 transformation;
	math::set_basis_scale_origin(transformation, math::float3x3(a.rotation), a.scale, a.position);

	world_frame_a_.position = local_frame_a_.transformation.position * transformation;
	world_frame_a_.rotation = local_frame_a_.transformation.rotation * a.rotation;
	world_frame_a_.scale    = local_frame_a_.transformation.scale;

	math::set_basis_scale_origin(transformation, math::float3x3(b.rotation), b.scale, b.position);

	world_frame_b_.position = local_frame_b_.transformation.position * transformation;
	world_frame_b_.rotation = local_frame_b_.transformation.rotation * b.rotation;
	world_frame_b_.scale    = local_frame_b_.transformation.scale;

	on_set_transformation(pool);

	propagate_transformation(pool);
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
//	if (node->parent_ == this) {
		node->parent_ = nullptr;

		if (child_ == node) {
			child_ = node->next_;
			node->next_ = nullptr;
		} else {
			child_->remove_sibling(node);
		}
//	}
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
