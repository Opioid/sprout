#pragma once

#include "base/math/vector.hpp"
#include "base/math/matrix.hpp"
#include "base/math/quaternion.hpp"
#include "base/math/transformation.hpp"

namespace scene { namespace entity {

struct Composed_transformation {
	void set(const math::transformation& t);

	math::float3 position;
	math::float3 scale;
	math::Matrix4x4f_a rotation;
	math::Matrix4x4f_a object_to_world;
	math::Matrix4x4f_a world_to_object;
};

}}
