#include "animation.hpp"
#include "scene/entity/keyframe.hpp"
#include "base/math/vector.inl"
#include "base/math/quaternion.inl"

namespace scene { namespace animation {

void Animation::reserve(size_t count) {
	keyframes_.clear();
	keyframes_.reserve(count);
}

void Animation::push_back(const entity::Keyframe& keyframe) {
	keyframes_.push_back(keyframe);
}

}}
