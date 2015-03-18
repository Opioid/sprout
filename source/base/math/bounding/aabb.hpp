#pragma once

#include "math/vector.hpp"
#include "math/matrix.hpp"
#include "math/ray.hpp"

namespace math {

class AABB {
public:

	AABB();
	AABB(const float3& min, const float3& max);

	void set_min_max(const float3& min, const float3& max);

	void transform(const float4x4& m, AABB& other) const;

	AABB merge(const AABB& other) const;

	static AABB empty();

private:

	float3 bounds_[2];
};

}
