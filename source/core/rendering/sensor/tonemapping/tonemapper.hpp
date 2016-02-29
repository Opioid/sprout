#pragma once

#include "base/math/vector.hpp"

namespace rendering { namespace sensor { namespace tonemapping {

class Tonemapper {
public:

	virtual ~Tonemapper();

	virtual math::vec3 tonemap(math::pvec3 color) const = 0;

protected:

	static math::vec3 white_factor(math::pvec3 linear_white, math::pvec3 tonemapped_white);
};

}}}
