#pragma once

#include "base/math/vector.hpp"
#include "base/math/matrix.hpp"
#include "base/math/transformation.hpp"

namespace scene { namespace entity {

struct alignas(16) Composed_transformation {
	void set(const math::transformation& t);

	math::Matrix4x4f_a world_to_object;
	math::Matrix4x4f_a object_to_world;
	math::Matrix3x3f_a rotation;
	math::Vector3f_a   position;
	math::Vector3f_a   scale;
};

}}
