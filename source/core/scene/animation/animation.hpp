#ifndef SU_CORE_SCENE_ANIMATION_ANIMATION_HPP
#define SU_CORE_SCENE_ANIMATION_ANIMATION_HPP

#include "base/math/transformation.hpp"

namespace thread {
class Pool;
}

using Threads = thread::Pool;

namespace scene {

class Scene;

namespace animation {

struct Morphing {
    void interpolate(Morphing& __restrict result, Morphing const& __restrict other, float t) const;

    uint32_t targets[2];
    float    weight;
};

struct Keyframe {
    math::Transformation k;

    Morphing m;

    uint64_t time;
};

class Animation {
  public:
    Animation(uint32_t entity, uint32_t num_frames, uint32_t num_interpolated_frames);

    Animation(Animation&& other);

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
