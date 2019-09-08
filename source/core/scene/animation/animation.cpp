#include "animation.hpp"
#include "base/math/quaternion.inl"
#include "base/math/transformation.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "scene/entity/keyframe.hpp"
#include "scene/scene.hpp"

namespace scene::animation {

Animation::Animation(uint32_t num_frames, uint32_t num_interpolated_frames) noexcept
    : last_frame_(0),
      num_keyframes_(num_frames),
      num_interpolated_frames_(num_interpolated_frames),
      keyframes_(memory::allocate_aligned<Keyframe>(num_frames + num_interpolated_frames)) {}

Animation::~Animation() noexcept {
    memory::free_aligned(keyframes_);
}

void Animation::set(uint32_t index, Keyframe const& keyframe) noexcept {
    keyframes_[index] = keyframe;
}

void Animation::resample(uint64_t start, uint64_t end, uint64_t frame_length) noexcept {
    uint64_t time = start;

    uint32_t const keyframes_back = num_keyframes_ - 1;

    uint32_t last_frame = last_frame_ > 2 ? last_frame_ - 2 : 0;

    Keyframe* interpolated_frames = &keyframes_[num_keyframes_];

    for (uint32_t i = 0; time <= end; ++i, time += frame_length) {
        for (uint32_t j = last_frame; j < keyframes_back; ++j) {
            auto const& a = keyframes_[j];
            auto const& b = keyframes_[j + 1];

            if (time >= a.k.time && time < b.k.time) {
                uint64_t const range = b.k.time - a.k.time;
                uint64_t const delta = time - a.k.time;

                float const t = float(delta) / float(range);

                a.k.interpolate(interpolated_frames[i].k, b.k, t);
                a.m.interpolate(interpolated_frames[i].m, b.m, t);

                interpolated_frames[i].k.time = time;

                break;
            } else if (j + 1 == keyframes_back) {
                interpolated_frames[i]        = b;
                interpolated_frames[i].k.time = time;

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

Keyframe const* Animation::interpolated_frames() const noexcept {
    return &keyframes_[num_keyframes_];
}

Stage::Stage(uint32_t entity, Animation* animation) noexcept
    : entity_(entity), animation_(animation) {}

void Stage::allocate_enitity_frames(Scene& scene) const noexcept {
    uint32_t const num_frames = animation_->num_interpolated_frames();
    scene.prop_allocate_frames(entity_, num_frames, num_frames);
}

void Stage::update(Scene& scene) const noexcept {
    scene.prop_set_frames(entity_, animation_->interpolated_frames(),
                          animation_->num_interpolated_frames());
}

}  // namespace scene::animation
