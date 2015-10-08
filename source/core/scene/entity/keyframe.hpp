#pragma once

#include "base/math/vector.hpp"
#include "base/math/quaternion.hpp"
#include "base/math/transformation.hpp"

namespace scene { namespace entity {

struct Keyframe {
	void interpolate(const Keyframe& other, float t, Keyframe& result) const;

	float time;
	math::transformation transformation;

	struct Morphing {
		uint32_t targets[2];
		float    weight;
	};

	Morphing morphing;
};

}}
