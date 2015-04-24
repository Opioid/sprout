#include "animation.hpp"
#include "keyframe.hpp"
#include "base/math/vector.inl"
#include "base/math/quaternion.inl"

namespace scene {

void Animation::reserve(size_t count) {
	keyframes_.clear();
	keyframes_.reserve(count);
}

void Animation::push_back(const Keyframe& keyframe) {
	keyframes_.push_back(keyframe);
}

}
