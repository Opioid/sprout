#include "animation.hpp"
#include "scene/entity/keyframe.hpp"
#include "base/math/vector.inl"
#include "base/math/quaternion.inl"
#include "base/math/transformation.inl"

#include <iostream>

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

	size_t max_frame = keyframes_.size() - 1;
	float next_frame_time = keyframes_[std::min(current_frame_ + 1, max_frame)].time;

	if (current_time_ > next_frame_time) {
		++current_frame_;
	}

	auto& current_frame = keyframes_[std::min(current_frame_,     max_frame)];
	auto& next_frame	= keyframes_[std::min(current_frame_ + 1, max_frame)];

	float range = next_frame.time - current_frame.time;

	float delta = current_time_ - current_frame.time;

	float t = delta / range;

	current_frame.interpolate(next_frame, t, interpolated_frame_);
}

void Animation::seek(float time) {
	current_time_ = time;

	current_frame_ = 0;

	for (size_t i = 0, len = keyframes_.size() - 1; i < len; ++i) {
		float next_frame_time = keyframes_[current_frame_ + 1].time;

		if (current_time_ <= next_frame_time) {
			break;
		}

		++current_frame_;
	}

	tick(0.f);
}

const entity::Keyframe& Animation::beginning() const {
	return keyframes_[0];
}

const entity::Keyframe& Animation::interpolated_frame() const {
	return interpolated_frame_;
}

}}
