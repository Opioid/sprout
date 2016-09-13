#pragma once

#include "base/math/vector.hpp"

namespace rendering { namespace sensor { namespace tonemapping {

class Tonemapper {

public:

	virtual ~Tonemapper();

	virtual float3 tonemap(float3_p color) const = 0;

protected:

	static float3 white_factor(float3_p linear_white, float3_p tonemapped_white);
};

}}}
