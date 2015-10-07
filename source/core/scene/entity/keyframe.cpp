#include "keyframe.hpp"
#include "base/math/transformation.inl"

namespace scene { namespace entity {

void Keyframe::interpolate(const Keyframe& other, float t, Keyframe& result) const {
	result.transformation = math::lerp(transformation, other.transformation, t);
}

}}
