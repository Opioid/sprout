#include "animation.hpp"
#include "base/math/quaternion.inl"
#include "base/math/transformation.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "scene/entity/keyframe.hpp"
#include "scene/scene.inl"
#include "scene/shape/morphable_shape.hpp"

namespace scene::animation {

Animation::Animation(uint32_t num_frames, uint32_t num_interpolated_frames) noexcept
    : last_frame_(0),
      num_keyframes_(num_frames),
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

            if (time >= a.time && time < b.time) {
                uint64_t const range = b.time - a.time;
                uint64_t const delta = time - a.time;

                float const t = float(delta) / float(range);

                a.k.interpolate(interpolated_frames[i].k, b.k, t);
                a.m.interpolate(interpolated_frames[i].m, b.m, t);

                break;
            } else if (j + 1 == keyframes_back) {
                interpolated_frames[i] = b;

                break;
            } else {
                ++last_frame;
            }
        }
    }

    last_frame_ = last_frame;
}

Keyframe const* Animation::interpolated_frames() const noexcept {
    return &keyframes_[num_keyframes_];
}

Stage::Stage(uint32_t entity, Animation* animation) noexcept
    : entity_(entity), animation_(animation) {}

void Stage::update(Scene& scene, thread::Pool& threads) const noexcept {
    scene.prop_set_frames(entity_, animation_->interpolated_frames());

    if (shape::Morphable_shape* morphable = scene.prop_shape(entity_)->morphable_shape();
        morphable) {
        auto const& m = animation_->interpolated_frames()[0].m;
        morphable->morph(m.targets[0], m.targets[1], m.weight, threads);
    }
}

}  // namespace scene::animation
