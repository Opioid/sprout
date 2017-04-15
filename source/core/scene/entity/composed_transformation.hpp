#pragma once

#include "base/math/vector3.hpp"
#include "base/math/matrix3x3.hpp"
#include "base/math/matrix4x4.hpp"
#include "base/math/transformation.hpp"

namespace scene { namespace entity {

struct Composed_transformation {
	void set(const math::Transformation& t);

	float4x4 world_to_object;
	float4x4 object_to_world;
	float3x3 rotation;
	float3   position;
	float3   scale;
};

}}
