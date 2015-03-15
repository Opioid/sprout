#pragma once

#include "base/math/vector.hpp"
#include "base/math/ray.hpp"

namespace scene { namespace surrounding {

class Surrounding {
public:

	virtual ~Surrounding();

	virtual math::float3 sample(const math::Oray& ray) const = 0;
};

}}
