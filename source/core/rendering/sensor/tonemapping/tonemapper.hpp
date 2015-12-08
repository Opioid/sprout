#pragma once

#include "base/math/vector.hpp"

namespace rendering { namespace sensor { namespace tonemapping {

class Tonemapper {
public:

	virtual ~Tonemapper() {}

	virtual math::float3 tonemap(const math::float3& color) const = 0;
};

}}}
