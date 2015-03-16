#pragma once

#include "math/vector.hpp"
#include "math/ray.hpp"

namespace math {

class AABB {
public:

	AABB();
	AABB(const float3& min, const float3& max);

private:

	float3 bounding_[2];
};

}
