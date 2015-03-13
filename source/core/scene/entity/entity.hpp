#pragma once

#include "composed_transformation.hpp"

namespace scene {

class Entity {
public:

	void transformation_at(float time, Composed_transformation& transformation);

private:

	Composed_transformation transformation_;
};

}
