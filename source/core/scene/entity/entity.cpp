#include "entity.hpp"
#include "base/math/matrix.inl"
#include "base/math/transformation.inl"

namespace scene { namespace entity {

bool Entity::transformation_at(float tick_delta, Composed_transformation& transformation) const {
	if (!animated_) {
		transformation = world_transformation_;
	} else {
		transformation.set(math::lerp(frame_a_.transformation, frame_b_.transformation, tick_delta));
	}

	return animated_;
}

void Entity::set_transformation(const math::transformation& t) {
	local_transformation_ = t;

	world_transformation_.set(t);

	animated_ = false;

	on_set_transformation();
}

void Entity::set_beginning(const Keyframe& frame) {
	frame_b_ = frame;
}

void Entity::tick(const Keyframe& frame) {
	frame_a_ = frame_b_;
	frame_b_ = frame;

	animated_ = true;

	on_set_transformation();
}

}}
