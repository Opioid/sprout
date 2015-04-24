#pragma once

#include "base/math/vector3.hpp"
#include "base/math/quaternion.hpp"
#include "base/math/transformation.hpp"

namespace scene {

struct Keyframe {
	float time;
	math::transformation transformation;
};

}
