#include "animation.hpp"
#include "base/math/quaternion.inl"
#include "base/math/transformation.inl"
#include "base/math/vector3.inl"
#include "scene/scene.inl"
#include "scene/shape/morphable.hpp"

namespace scene::animation {

static inline Morphing lerp(Morphing const& a, Morphing const& b, float t) {
    if (a.targets[0] == a.targets[1] && b.targets[0] == b.targets[1]) {
        return {{a.targets[0], b.targets[0]}, t};
    } else {
        return {{b.targets[0], b.targets[1]}, ::lerp(a.weight, b.weight, t)};
    }
}

Animation::Animation(uint32_t entity, uint32_t num_frames, uint32_t num_interpolated_frames)
    : entity_(entity),
      last_frame_(0),
      num_keyframes_(num_frames),
      times_(new uint64_t[num_frames]),
      frames_(new math::Transformation[num_frames + num_interpolated_frames]),
      morphings_(new Morphing[num_frames + num_interpolated_frames]) {}

Animation::Animation(Animation&& other)
    : entity_(other.entity_),
      last_frame_(other.last_frame_),
      num_keyframes_(other.num_keyframes_),
      times_(other.times_),
      frames_(other.frames_),
      morphings_(other.morphings_) {
    other.times_     = nullptr;
    other.frames_    = nullptr;
    other.morphings_ = nullptr;
}

Animation::~Animation() {
    delete[] morphings_;
    delete[] frames_;
    delete[] times_;
}

void Animation::set(uint32_t index, Keyframe const& keyframe) {
    times_[index]     = keyframe.time;
    frames_[index]    = keyframe.k;
    morphings_[index] = keyframe.m;
}

void Animation::resample(uint64_t start, uint64_t end, uint64_t frame_length) {
    uint64_t time = start;

    uint32_t const keyframes_back = num_keyframes_ - 1;

    uint32_t last_frame = last_frame_ > 2 ? last_frame_ - 2 : 0;

    math::Transformation* interpolated_frames    = &frames_[num_keyframes_];
    Morphing*             interpolated_morphings = &morphings_[num_keyframes_];

    for (uint32_t i = 0; time <= end; ++i, time += frame_length) {
        for (uint32_t j = last_frame; j < keyframes_back; ++j) {
            uint64_t const a_time = times_[j];
            uint64_t const b_time = times_[j + 1];

            auto const& a_frame = frames_[j];
            auto const& b_frame = frames_[j + 1];

            auto const& a_morphing = morphings_[j];
            auto const& b_morphing = morphings_[j + 1];

            if (time >= a_time && time < b_time) {
                uint64_t const range = b_time - a_time;
                uint64_t const delta = time - a_time;

                float const t = float(double(delta) / double(range));

                interpolated_frames[i]    = lerp(a_frame, b_frame, t);
                interpolated_morphings[i] = lerp(a_morphing, b_morphing, t);

                break;
            }

            if (j + 1 == keyframes_back) {
                interpolated_frames[i] = b_frame;

                break;
            }

            ++last_frame;
        }
    }

    last_frame_ = last_frame;
}

void Animation::update(Scene& scene, Threads& threads) const {
    math::Transformation const* interpolated = &frames_[num_keyframes_];

    scene.prop_set_frames(entity_, interpolated);

    if (shape::Morphable* morphable = scene.prop_shape(entity_)->morphable_shape(); morphable) {
        Morphing const* im = &morphings_[num_keyframes_];

        morphable->morph(im, scene.num_interpolation_frames(), threads);
    }
}

}  // namespace scene::animation
