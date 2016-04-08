#pragma once

#include "hitpoint.hpp"

namespace scene { namespace shape {

struct Intersection : public Hitpoint {
	float epsilon;
	uint32_t part;
};

}}
