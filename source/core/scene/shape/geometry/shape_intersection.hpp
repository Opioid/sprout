#pragma once

#include "differential.hpp"

namespace scene { namespace shape {

struct Intersection : public Differential {
	float epsilon;
	uint32_t material_index;
};

}}
