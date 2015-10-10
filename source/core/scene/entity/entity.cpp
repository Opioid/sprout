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
		transformation.set(math::lerp(frame_a_.transformation, frame_b_.transformation, tick_delta));
	}

	return animated_;
}

void Entity::set_transformation(const math::transformation& t, thread::Pool& pool) {
	world_transformation_.set(t);

	animated_ = false;

	on_set_transformation(pool);
}

void Entity::set_beginning(const Keyframe& frame) {
	frame_b_ = frame;
}

void Entity::tick(const Keyframe& frame, thread::Pool& pool) {
	frame_a_ = frame_b_;
	frame_b_ = frame;

	animated_ = true;

	on_set_transformation(pool);
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
