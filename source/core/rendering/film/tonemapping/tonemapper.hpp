#pragma once

#include "base/math/vector.hpp"

namespace rendering { namespace film { namespace tonemapping {

class Tonemapper {
public:

	virtual math::float3 tonemap(const math::float3& color) const = 0;
};

}}}
