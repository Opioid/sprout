#ifndef SU_CORE_SCENE_SHAPE_MORPHABLE_SHAPE_HPP
#define SU_CORE_SCENE_SHAPE_MORPHABLE_SHAPE_HPP

#include <cstdint>

namespace thread {
class Pool;
}

using Threads = thread::Pool;

namespace scene {

namespace entity {
struct Morphing;
}

namespace shape {

class Morphable {
  public:
    using Morphing = entity::Morphing;

    virtual ~Morphable() {}

    virtual void morph(Morphing const& a, Threads& threads) = 0;
};

}  // namespace scene::shape
}

#endif
