#ifndef SU_CORE_SCENE_ANIMATION_ANIMATION_HPP
#define SU_CORE_SCENE_ANIMATION_ANIMATION_HPP

#include <cstddef>
#include <vector>
#include "scene/entity/keyframe.hpp"

namespace scene::animation {

class Animation {
  public:
    Animation() = default;

    ~Animation();

    void init(uint32_t count) noexcept;

    void allocate_interpolated_frames(uint32_t num_frames) noexcept;

    void push_back(entity::Keyframe const& keyframe) noexcept;

    void resample(uint64_t start, uint64_t end, uint64_t frame_length) noexcept;

    entity::Keyframe const& interpolated_frame() const noexcept;

    uint32_t num_interpolated_frames() const noexcept;

    entity::Keyframe const* interpolated_frames() const noexcept;

  private:
    uint32_t last_frame_;

    std::vector<entity::Keyframe> keyframes_;

    uint32_t num_interpolated_frames_ = 0;

    entity::Keyframe* interpolated_frames_ = nullptr;
};

}  // namespace scene::animation

#endif
