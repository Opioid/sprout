#include "animation.hpp"
#include "base/math/quaternion.inl"
#include "base/math/transformation.inl"
#include "base/math/vector3.inl"
#include "scene/entity/keyframe.hpp"

namespace scene::animation {

Animation::~Animation() {
    delete[] interpolated_frames_;
}

void Animation::init(uint32_t count) noexcept {
    current_time_  = 0.f;
    current_frame_ = 0;
    last_frame_ = 0;
    keyframes_.clear();
    keyframes_.reserve(count);
}

void Animation::allocate_interpolated_frames(uint32_t num_frames) noexcept {
    num_interpolated_frames_ = num_frames;

    interpolated_frames_ = new entity::Keyframe[num_frames];
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
        current_frame.interpolate(interpolated_frame_, next_frame, delta);
    } else {
        float const t = delta / range;

        current_frame.interpolate(interpolated_frame_, next_frame, t);
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

void Animation::resample(uint64_t start, uint64_t frame_length, uint32_t num_frames) noexcept {
    uint64_t time = start;

    for (uint32_t i = 0; i <= num_frames; ++i, time += frame_length) {
        for (uint32_t j = last_frame_, len = static_cast<uint32_t>(keyframes_.size()) - 1; j < len; ++j) {
            auto const& a = keyframes_[j];
            auto const& b = keyframes_[j + 1];

            if (time >= a.time_i && time < b.time_i) {
                uint64_t const range = b.time_i - a.time_i;
                uint64_t const delta = time - a.time_i;

                float const t = static_cast<float>(delta) / static_cast<float>(range);

                a.interpolate(interpolated_frames_[i], b, t);
                interpolated_frames_[i].time_i = time;

                break;
            }

            ++last_frame_;
        }
    }
}

entity::Keyframe const& Animation::interpolated_frame() const noexcept {
    return interpolated_frame_;
}

uint32_t Animation::num_interpolated_frames() const noexcept {
    return num_interpolated_frames_;
}

entity::Keyframe const* Animation::interpolated_frames() const noexcept {
    return interpolated_frames_;
}

}  // namespace scene::animation
