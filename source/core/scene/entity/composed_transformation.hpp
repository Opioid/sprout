#pragma once

#include "base/math/vector3.hpp"
#include "base/math/matrix3x3.hpp"
#include "base/math/matrix4x4.hpp"
#include "base/math/transformation.hpp"

namespace scene { namespace entity {

struct Composed_transformation {
	void set(const math::Transformation& t);

	math::Matrix4x4f_a world_to_object;
	math::Matrix4x4f_a object_to_world;
	math::Matrix3x3f_a rotation;
	math::Vector3f_a   position;
	math::Vector3f_a   scale;
};

}}
