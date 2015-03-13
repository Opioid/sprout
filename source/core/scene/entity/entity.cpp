#include "entity.hpp"
#include "base/math/matrix.inl"

namespace scene {

void Entity::transformation_at(float time, Composed_transformation& transformation) const {
	transformation = transformation_;
}

}
