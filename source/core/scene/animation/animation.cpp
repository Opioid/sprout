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

	float next_frame_time = keyframes_[current_frame_ + 1].time;

	if (current_time_ > next_frame_time) {
		++current_frame_;
	}

	size_t max_frame = keyframes_.size() - 1;
	auto& current_frame = keyframes_[std::min(current_frame_,     max_frame)];
	auto& next_frame	= keyframes_[std::min(current_frame_ + 1, max_frame)];

	float range = next_frame.time - current_frame.time;

	float delta = current_time_ - current_frame.time;

	float t = delta / range;

	current_frame.interpolate(next_frame, t, interpolated_frame_);
}

void Animation::beginning(entity::Keyframe& frame) const {
	frame = keyframes_[0];
}

void Animation::current_frame(entity::Keyframe& frame) const {
	frame = interpolated_frame_;
}

}}
