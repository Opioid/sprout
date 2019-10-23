#ifndef SU_CORE_BAKING_DRIVER_HPP
#define SU_CORE_BAKING_DRIVER_HPP

#include <cstdint>

namespace take {
struct Take;
struct View;
}  // namespace take

namespace scene {

namespace camera {
class Camera;
}

class Scene;

}  // namespace scene

namespace sampler {
class Factory;
}

namespace thread {
class Pool;
}

namespace rendering {

namespace integrator {

namespace surface {
class Factory;
}
namespace volume {
class Factory;
}

}  // namespace integrator
}  // namespace rendering

namespace progress {
class Sink;
}

namespace baking {

class Driver {
  public:
    using Scene  = scene::Scene;
    using Camera = scene::camera::Camera;

    Driver(take::Take& take, Scene& scene, thread::Pool& thread_pool,
           uint32_t max_material_sample_size, progress::Sink& progressor) noexcept;

    ~Driver() noexcept;

    void render() noexcept;

  private:
    progress::Sink& progressor_;
};

}  // namespace baking

#endif
