#ifndef SU_CORE_SCENE_ANIMATION_ANIMATION_HPP
#define SU_CORE_SCENE_ANIMATION_ANIMATION_HPP

#include <cstddef>
#include <vector>
#include "scene/entity/keyframe.hpp"

namespace scene::animation {

class Animation {
  public:
    void init(uint32_t count) noexcept;

    void push_back(entity::Keyframe const& keyframe) noexcept;

    void tick(float time_slice) noexcept;

    void seek(float time) noexcept;

    void simulate(uint64_t begin, uint64_t end) noexcept;

    entity::Keyframe const& interpolated_frame() const noexcept;

    uint32_t num_interpolated_frames() const noexcept;

    entity::Keyframe const* interpolated_frames() const noexcept;

  private:
    float current_time_;

    uint32_t current_frame_;

    std::vector<entity::Keyframe> keyframes_;

    entity::Keyframe interpolated_frame_;

    uint32_t num_frames_;

    entity::Keyframe* interpolated_frames_;
};

}  // namespace scene::animation

#endif
