#include "animation.hpp"
#include "scene/entity/keyframe.hpp"
#include "base/math/vector.inl"
#include "base/math/quaternion.inl"
#include "base/math/transformation.inl"

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


void Animation::tick(float time_slice) {
	current_time_ += time_slice;

	auto& current_frame = keyframes_[current_frame_];
	auto& next_frame = keyframes_[current_frame_ + 1];
//	if (current_time_ <= next_frame.time) {

//	}

	float range = next_frame.time - current_frame.time;

	float delta = current_time_ - current_frame.time;

	float t = delta / range;

	interpolated_frame_ = math::lerp(current_frame.transformation, next_frame.transformation, t);
}

void Animation::beginning(math::transformation& t) const {
	t = keyframes_[0].transformation;
}

void Animation::current_frame(math::transformation& t) const {
	t = interpolated_frame_;
}

}}
