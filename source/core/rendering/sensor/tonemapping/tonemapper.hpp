#pragma once

#include "base/math/vector.hpp"

namespace rendering { namespace sensor { namespace tonemapping {

class Tonemapper {
public:

	virtual ~Tonemapper();

	virtual math::float3 tonemap(math::pfloat3 color) const = 0;

protected:

	static math::float3 white_factor(math::pfloat3 linear_white, math::pfloat3 tonemapped_white);
};

}}}
