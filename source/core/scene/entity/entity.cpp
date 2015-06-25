#include "entity.hpp"
#include "base/math/matrix.inl"
#include "base/math/transformation.inl"

namespace scene { namespace entity {

bool Entity::transformation_at(float tick_delta, Composed_transformation& transformation) const {
	if (!animated_) {
		transformation = world_transformation_;
	} else {
		math::lerp(world_transformation_a_, world_transformation_b_, tick_delta);
	}

	return animated_;
}

void Entity::set_transformation(const math::transformation& t) {
	local_transformation_ = t;

	world_transformation_.set(t);

	animated_ = false;

	on_set_transformation();
}

void Entity::set_tick_transformation(const math::transformation& a, const math::transformation& b) {
	world_transformation_b_ = a;
	world_transformation_a_ = b;

	animated_ = true;

	on_set_transformation();
}

}}
