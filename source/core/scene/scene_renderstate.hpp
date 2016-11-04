#pragma once

#include "shape/geometry/hitpoint.hpp"

namespace scene {

struct Renderstate : public shape::Hitpoint {
	float area;
	float time;
	float ior;
};

}
