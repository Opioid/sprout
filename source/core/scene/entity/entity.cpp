#include "entity.hpp"
#include "base/math/matrix.inl"

namespace scene { namespace entity {

bool Entity::transformation_at(float /*time*/, Composed_transformation& transformation) const {
	transformation = transformation_;

	return false;
}

void Entity::set_transformation(const math::transformation& t) {
	transformation_.set(t);

	on_set_transformation();
}

void Entity::on_set_transformation() {

}

}}
