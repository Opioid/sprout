#ifndef SU_CORE_SCENE_ANIMATION_STAGE_HPP
#define SU_CORE_SCENE_ANIMATION_STAGE_HPP

#include <cstdint>

namespace scene {

class Scene;

namespace animation {

class Animation;

class Stage {
  public:
    Stage(uint32_t entity, Animation* animation) noexcept;

    void allocate_enitity_frames(Scene const& scene) const noexcept;

    void update(Scene const& scene) const noexcept;

  private:
    uint32_t entity_;

    Animation* animation_;
};

}  // namespace animation
}  // namespace scene

#endif
