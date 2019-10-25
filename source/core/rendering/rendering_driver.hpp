#ifndef SU_CORE_RENDERING_DRIVER_HPP
#define SU_CORE_RENDERING_DRIVER_HPP

#define PARTICLE_TRAINING
#define PHOTON_TRAINING

#include "image/typed_image.hpp"
#include "image/typed_image_fwd.hpp"
#include "integrator/particle/particle_importance.hpp"
#include "integrator/particle/photon/photon_map.hpp"
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
    using Scene  = scene::Scene;
    using Camera = scene::camera::Camera;

    Driver(take::Take& take, Scene& scene, thread::Pool& threads,
           uint32_t max_sample_size) noexcept;

    ~Driver() noexcept;

    Camera& camera() noexcept;

    Scene const& scene() const noexcept;
    Scene&       scene() noexcept;

    size_t num_bytes() const noexcept;

  protected:
    Scene& scene_;

    take::View& view_;

    thread::Pool& threads_;

    Camera_worker* workers_;

    uint32_t frame_;
    uint32_t iteration_;

    Tile_queue tiles_;

    Range_queue ranges_;

    image::Float4 target_;

    take::Photon_settings photon_settings_;

    integrator::particle::photon::Map photon_map_;

    integrator::particle::Importance_cache particle_importance_;

    struct Photon_info {
        uint32_t num_paths;
    };

    Photon_info* photon_infos_;
};

}  // namespace rendering

#endif
