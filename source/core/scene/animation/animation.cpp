#include "animation.hpp"
#include "base/math/quaternion.inl"
#include "base/math/transformation.inl"
#include "base/math/vector3.inl"
#include "scene/entity/keyframe.hpp"

namespace scene::animation {

void Animation::init(uint32_t count) noexcept {
    current_time_  = 0.f;
    current_frame_ = 0;
    keyframes_.clear();
    keyframes_.reserve(count);
}

void Animation::push_back(entity::Keyframe const& keyframe) noexcept {
    keyframes_.push_back(keyframe);
}

void Animation::tick(float time_slice) noexcept {
    current_time_ += time_slice;

    uint32_t const max_frame = static_cast<uint32_t>(keyframes_.size() - 1);

    float const next_frame_time = keyframes_[std::min(current_frame_ + 1, max_frame)].time;

    if (current_time_ > next_frame_time) {
        ++current_frame_;
    }

    uint32_t const current_frame_id = std::min(current_frame_, max_frame);
    uint32_t const next_frame_id    = std::min(current_frame_ + 1, max_frame);

    auto const& current_frame = keyframes_[current_frame_id];
    auto const& next_frame    = keyframes_[next_frame_id];

    float const range = next_frame.time - current_frame.time;

    float const delta = current_time_ - current_frame.time;

    if (range <= 0.f) {
        current_frame.interpolate(next_frame, delta, interpolated_frame_);
    } else {
        float t = delta / range;

        current_frame.interpolate(next_frame, t, interpolated_frame_);
    }
}

void Animation::seek(float time) noexcept {
    current_time_ = time;

    current_frame_ = 0;

    for (size_t i = 0, len = keyframes_.size() - 1; i < len; ++i) {
        float const next_frame_time = keyframes_[current_frame_ + 1].time;

        if (current_time_ <= next_frame_time) {
            break;
        }

        ++current_frame_;
    }

    tick(0.f);
}

entity::Keyframe const& Animation::beginning() const noexcept {
    return keyframes_[0];
}

entity::Keyframe const& Animation::interpolated_frame() const noexcept {
    return interpolated_frame_;
}

}  // namespace scene::animation
