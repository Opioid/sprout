#include "entity.hpp"

namespace scene {

void Entity::transformation_at(float time, Composed_transformation& transformation) {
	transformation = transformation_;
}

}
