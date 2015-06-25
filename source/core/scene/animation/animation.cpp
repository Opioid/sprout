#include "animation.hpp"
#include "scene/entity/keyframe.hpp"
#include "base/math/vector.inl"
#include "base/math/quaternion.inl"

namespace scene { namespace animation {

void Animation::init(size_t count) {
    current_time_ = 0.f;
    current_frame_ = 0;
	keyframes_.clear();
	keyframes_.reserve(count);
}

void Animation::push_back(const entity::Keyframe& keyframe) {
	keyframes_.push_back(keyframe);
}

void Animation::beginning(entity::Keyframe& keyframe) const {
    keyframe = keyframes_[0];
}

void Animation::tick(float time_slice, entity::Keyframe& keyframe) {

}

}}
