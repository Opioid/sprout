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

class Baking_worker;

class Driver {
  public:
    using Scene  = scene::Scene;
    using Camera = scene::camera::Camera;

    Driver(take::Take& take, Scene& scene, thread::Pool& thread_pool, uint32_t max_sample_size,
           progress::Sink& progressor) noexcept;

    ~Driver() noexcept;

    void render() noexcept;

  private:
    Scene& scene_;

    take::View& view_;

    thread::Pool& thread_pool_;

    Baking_worker* workers_;

    progress::Sink& progressor_;
};

}  // namespace baking

#endif
