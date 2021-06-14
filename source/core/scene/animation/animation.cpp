#include "animation.hpp"
#include "base/math/quaternion.inl"
#include "base/math/transformation.inl"
#include "base/math/vector3.inl"
#include "scene/entity/keyframe.hpp"
#include "scene/scene.inl"
#include "scene/shape/morphable.hpp"

namespace scene::animation {

Animation::Animation(uint32_t entity, uint32_t num_frames, uint32_t num_interpolated_frames)
    : entity_(entity),
      last_frame_(0),
      num_keyframes_(num_frames),
      keyframes_(new Keyframe[num_frames + num_interpolated_frames]) {}

Animation::Animation(Animation&& other)
    : entity_(other.entity_),
      last_frame_(other.last_frame_),
      num_keyframes_(other.num_keyframes_),
      keyframes_(other.keyframes_) {
    other.keyframes_ = nullptr;
}

Animation::~Animation() {
    delete[] keyframes_;
}

void Animation::set(uint32_t index, Keyframe const& keyframe) {
    keyframes_[index] = keyframe;
}

void Animation::resample(uint64_t start, uint64_t end, uint64_t frame_length) {
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

                float const t = float(double(delta) / double(range));

                a.k.interpolate(interpolated_frames[i].k, b.k, t);
                a.m.interpolate(interpolated_frames[i].m, b.m, t);

                break;
            }

            if (j + 1 == keyframes_back) {
                interpolated_frames[i] = b;

                break;
            }

            ++last_frame;
        }
    }

    last_frame_ = last_frame;
}

void Animation::update(Scene& scene, Threads& threads) const {
    Keyframe const* interpolated = &keyframes_[num_keyframes_];

    scene.prop_set_frames(entity_, interpolated);

    if (shape::Morphable* morphable = scene.prop_shape(entity_)->morphable_shape(); morphable) {
        morphable->morph(interpolated, scene.num_interpolation_frames(), threads);
    }
}

}  // namespace scene::animation
