#include "aabb.hpp"
#include "math/vector.inl"

namespace math {

AABB::AABB() {}

AABB::AABB(const float3& min, const float3& max) {
	bounding_[0] = min;
	bounding_[1] = max;
}

}
