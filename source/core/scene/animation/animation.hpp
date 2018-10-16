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

    entity::Keyframe const& beginning() const noexcept;

    entity::Keyframe const& interpolated_frame() const noexcept;

  private:
    float current_time_;

    uint32_t current_frame_;

    std::vector<entity::Keyframe> keyframes_;

    entity::Keyframe interpolated_frame_;
};

}  // namespace scene::animation

#endif
