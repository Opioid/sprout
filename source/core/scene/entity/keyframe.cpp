#include "keyframe.hpp"
#include "base/math/math.hpp"
#include "base/math/transformation.inl"

namespace scene { namespace entity {

void Keyframe::interpolate(const Keyframe& other, float t, Keyframe& result) const {
	result.transformation = math::lerp(transformation, other.transformation, t);

	if (morphing.targets[0]       == morphing.targets[1]
	&&  other.morphing.targets[0] == other.morphing.targets[1]) {
		result.morphing.weight = t;

		result.morphing.targets[0] = morphing.targets[0];
		result.morphing.targets[1] = other.morphing.targets[0];
	}
/*	else if (morphing.targets[0] != other.morphing.targets[0]
		 ||  morphing.targets[1] != other.morphing.targets[1]) {
		result.morphing.weight = other.morphing.weight;
	}*/ else {
		result.morphing.weight = math::lerp(morphing.weight, other.morphing.weight, t);

		result.morphing.targets[0] = other.morphing.targets[0];
		result.morphing.targets[1] = other.morphing.targets[1];
	}


}

}}
