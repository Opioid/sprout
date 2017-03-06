#pragma once

#include "base/math/transformation.hpp"

namespace scene { namespace entity {

struct alignas(16) Keyframe {
	void interpolate(const Keyframe& other, float t, Keyframe& result) const;

	math::Transformation transformation;

	struct Morphing {
		uint32_t targets[2];
		float    weight;
	};

	Morphing morphing;

	float time;
};

}}
