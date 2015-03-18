#include "entity.hpp"
#include "base/math/matrix.inl"

namespace scene {

void Entity::transformation_at(float time, Composed_transformation& transformation) const {
	transformation = transformation_;
}

void Entity::set_transformation(const math::float3& position, const math::float3& scale, const math::quaternion& rotation) {
	transformation_.set(position, scale, rotation);

	on_set_transformation();
}

void Entity::on_set_transformation() {

}

}
