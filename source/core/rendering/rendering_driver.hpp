#ifndef SU_CORE_RENDERING_DRIVER_HPP
#define SU_CORE_RENDERING_DRIVER_HPP

#include <memory>
#include "image/typed_image.hpp"
#include "image/typed_image_fwd.hpp"
#include "tile_queue.hpp"

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

class Camera_worker;

class Driver {
  public:
    Driver(take::Take& take, scene::Scene& scene, thread::Pool& thread_pool,
           uint32_t max_sample_size);

    ~Driver();

    scene::camera::Camera& camera();

    scene::Scene const& scene() const;

    size_t num_bytes() const;

  protected:
    scene::Scene& scene_;
    take::View&   view_;
    thread::Pool& thread_pool_;

    Camera_worker* workers_;
    Tile_queue     tiles_;

    image::Float4 target_;
};

}  // namespace rendering

#endif
