#pragma once

#include "base/math/vector.hpp"
#include "base/math/matrix.hpp"
#include "base/math/quaternion.hpp"

namespace scene {

struct Composed_transformation {
	void set(const math::float3& p, const math::float3& s, const math::quaternion& r);

	math::float3   position;
	math::float3   scale;
	math::float3x3 rotation;
	math::float4x4 object_to_world;
	math::float4x4 world_to_object;
};

}
