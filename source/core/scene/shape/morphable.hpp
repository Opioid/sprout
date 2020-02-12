#ifndef SU_CORE_SCENE_SHAPE_MORPHABLE_SHAPE_HPP
#define SU_CORE_SCENE_SHAPE_MORPHABLE_SHAPE_HPP

#include <cstdint>

namespace thread {
class Pool;
}

namespace scene::shape {

class Morphable {
  public:
    virtual ~Morphable() {}

    virtual void morph(uint32_t a, uint32_t b, float weight, thread::Pool& threads) = 0;
};

}  // namespace scene::shape

#endif