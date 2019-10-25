#ifndef SU_CORE_SCENE_ANIMATION_ANIMATION_HPP
#define SU_CORE_SCENE_ANIMATION_ANIMATION_HPP

#include "scene/entity/keyframe.hpp"

#include <cstdint>

namespace thread {
class Pool;
}

namespace scene {

class Scene;

namespace animation {

struct Keyframe {
    entity::Keyframe k;
    entity::Morphing m;
};

class Animation {
  public:
    Animation(uint32_t num_frames, uint32_t num_interpolated_frames) noexcept;

    ~Animation() noexcept;

    void set(uint32_t index, Keyframe const& keyframe) noexcept;

    void resample(uint64_t start, uint64_t end, uint64_t frame_length) noexcept;

    Keyframe const* interpolated_frames() const noexcept;

  private:
    uint32_t last_frame_;
    uint32_t num_keyframes_;

    Keyframe* keyframes_;
};

class Stage {
  public:
    Stage(uint32_t entity, Animation* animation) noexcept;

    void allocate_enitity_frames(Scene& scene) const noexcept;

    void update(Scene& scene, thread::Pool& threads) const noexcept;

  private:
    uint32_t entity_;

    Animation* animation_;
};

}  // namespace animation
}  // namespace scene

#endif
