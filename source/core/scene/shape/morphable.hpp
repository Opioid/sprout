#ifndef SU_CORE_SCENE_SHAPE_MORPHABLE_SHAPE_HPP
#define SU_CORE_SCENE_SHAPE_MORPHABLE_SHAPE_HPP

#include <cstdint>

namespace thread {
class Pool;
}

using Threads = thread::Pool;

namespace scene {

namespace animation {
struct Morphing;
}

namespace shape {

class Morphable {
  public:
    using Morphing = animation::Morphing;

    virtual ~Morphable() {}

    virtual void morph(Morphing const* morphings, uint32_t num_frames, Threads& threads) = 0;
};

}  // namespace shape
}  // namespace scene

#endif
