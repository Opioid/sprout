#pragma once

#include "base/math/vector.hpp"

namespace rendering { namespace film { namespace filter {

class Filter {
public:

	virtual ~Filter() {}

	virtual float evaluate(math::float2 p) const = 0;
};

}}}
