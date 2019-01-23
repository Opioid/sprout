#ifndef SU_CORE_SCENE_ANIMATION_ANIMATION_HPP
#define SU_CORE_SCENE_ANIMATION_ANIMATION_HPP

#include <cstdint>

namespace scene {

namespace entity {
struct alignas(16) Keyframe;
}

namespace animation {

class Animation {
  public:
    Animation(uint32_t count) noexcept;

    ~Animation() noexcept;

    void allocate_interpolated_frames(uint32_t num_frames) noexcept;

    void set(uint32_t index, entity::Keyframe const& keyframe) noexcept;

    void resample(uint64_t start, uint64_t end, uint64_t frame_length) noexcept;

    entity::Keyframe const& interpolated_frame() const noexcept;

    uint32_t num_interpolated_frames() const noexcept;

    entity::Keyframe const* interpolated_frames() const noexcept;

  private:
    uint32_t last_frame_ = 0;

    uint32_t num_keyframes_ = 0;

    entity::Keyframe* keyframes_ = nullptr;

    uint32_t num_interpolated_frames_ = 0;

    entity::Keyframe* interpolated_frames_ = nullptr;
};

}  // namespace animation
}  // namespace scene

#endif
