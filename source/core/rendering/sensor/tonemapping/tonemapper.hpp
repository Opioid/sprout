#pragma once

#include "base/math/vector.hpp"

namespace rendering { namespace sensor { namespace tonemapping {

class Tonemapper {

public:

	virtual ~Tonemapper();

	virtual float3 tonemap(float3_p color) const = 0;

protected:

	static float normalization_factor(float hdr_max, float tonemapped_max);
};

}}}
