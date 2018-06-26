#ifndef SU_CORE_RENDERING_DRIVER_HPP
#define SU_CORE_RENDERING_DRIVER_HPP

#include <memory>
#include "base/math/vector2.hpp"
#include "image/typed_image.hpp"
#include "image/typed_image_fwd.hpp"
#include "integrator/photon/photon_map.hpp"
#include "take/take_settings.hpp"
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
           uint32_t max_material_sample_size);

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

    integrator::photon::Map photon_map_;
    take::Photon_settings   photon_settings_;

    struct Photon_info {
        uint2    range;
        uint32_t num_paths;
    };

    Photon_info* photon_infos_;
};

}  // namespace rendering

#endif
