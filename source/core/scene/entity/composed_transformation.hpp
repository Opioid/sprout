#pragma once

#include "base/math/vector.hpp"
#include "base/math/matrix.hpp"

namespace scene {

struct Composed_transformation {
	math::float3   position;
	math::float3   scale;
	math::float3x3 rotation;
	math::float4x4 object_to_world;
	math::float4x4 world_to_object;
};

}
