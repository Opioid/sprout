#pragma once

#include "base/math/vector3.hpp"

namespace rendering { namespace sensor { namespace filter {

class Filter {

public:

	virtual ~Filter() {}

	virtual float radius() const = 0;

	virtual float evaluate(float d) const = 0;
	virtual float evaluate(float2 p) const = 0;
};

}}}
