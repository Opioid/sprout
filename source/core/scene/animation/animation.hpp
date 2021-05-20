#ifndef SU_CORE_SCENE_ANIMATION_ANIMATION_HPP
#define SU_CORE_SCENE_ANIMATION_ANIMATION_HPP

#include "scene/entity/keyframe.hpp"

#include <cstdint>

namespace thread {
class Pool;
}

using Threads = thread::Pool;

namespace scene {

class Scene;

namespace animation {

struct Keyframe {
    entity::Keyframe k;
    entity::Morphing m;

    uint64_t time;
};

class Animation {
  public:
    Animation(uint32_t entity, uint32_t num_frames, uint32_t num_interpolated_frames);

    ~Animation();

    void set(uint32_t index, Keyframe const& keyframe);

    void resample(uint64_t start, uint64_t end, uint64_t frame_length);

    void update(Scene& scene, Threads& threads) const;

  private:
    uint32_t entity_;

    uint32_t last_frame_;
    uint32_t num_keyframes_;

    Keyframe* keyframes_;
};

}  // namespace animation
}  // namespace scene

#endif
