#include "animation.hpp"
#include "base/math/quaternion.inl"
#include "base/math/transformation.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "scene/entity/keyframe.hpp"

namespace scene::animation {

Animation::Animation(uint32_t num_frames, uint32_t num_interpolated_frames) noexcept
    : last_frame_(0),
      num_keyframes_(num_frames),
      num_interpolated_frames_(num_interpolated_frames),
      keyframes_(memory::allocate_aligned<entity::Keyframe>(num_frames + num_interpolated_frames)) {
}

Animation::~Animation() noexcept {
    memory::free_aligned(keyframes_);
}

void Animation::set(uint32_t index, entity::Keyframe const& keyframe) noexcept {
    keyframes_[index] = keyframe;
}

void Animation::resample(uint64_t start, uint64_t end, uint64_t frame_length) noexcept {
    uint64_t time = start;

    uint32_t const keyframes_back = num_keyframes_ - 1;

    uint32_t last_frame = last_frame_ > 2 ? last_frame_ - 2 : 0;

    entity::Keyframe* interpolated_frames = &keyframes_[num_keyframes_];

    for (uint32_t i = 0; time <= end; ++i, time += frame_length) {
        for (uint32_t j = last_frame; j < keyframes_back; ++j) {
            auto const& a = keyframes_[j];
            auto const& b = keyframes_[j + 1];

            if (time >= a.time && time < b.time) {
                uint64_t const range = b.time - a.time;
                uint64_t const delta = time - a.time;

                float const t = static_cast<float>(delta) / static_cast<float>(range);

                a.interpolate(interpolated_frames[i], b, t);
                interpolated_frames[i].time = time;

                break;
            } else if (j + 1 == keyframes_back) {
                interpolated_frames[i]      = b;
                interpolated_frames[i].time = time;

                break;
            } else {
                ++last_frame;
            }
        }
    }

    last_frame_ = last_frame;
}

uint32_t Animation::num_interpolated_frames() const noexcept {
    return num_interpolated_frames_;
}

entity::Keyframe const* Animation::interpolated_frames() const noexcept {
    return &keyframes_[num_keyframes_];
}

}  // namespace scene::animation
