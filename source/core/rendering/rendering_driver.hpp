#ifndef SU_CORE_RENDERING_DRIVER_HPP
#define SU_CORE_RENDERING_DRIVER_HPP

#define PARTICLE_GUIDING
#define PHOTON_GUIDING

#include "base/memory/array.hpp"
#include "image/typed_image.hpp"
#include "image/typed_image_fwd.hpp"
#include "integrator/particle/particle_importance.hpp"
#include "integrator/particle/photon/photon_map.hpp"
#include "tile_queue.hpp"

namespace take {
struct View;
}  // namespace take

namespace scene {

namespace camera {
class Camera;
}

class Scene;

}  // namespace scene

namespace exporting {
class Sink;
}

namespace progress {
class Sink;
}

namespace rendering {

class Camera_worker;

class Driver {
  public:
    using Scene     = scene::Scene;
    using Camera    = scene::camera::Camera;
    using Exporters = memory::Array<exporting::Sink*>;

    Driver(thread::Pool& threads, progress::Sink& progressor);

    ~Driver();

    void init(take::View& view, Scene& scene, bool progressive);

    Camera& camera();

    Scene const& scene() const;
    Scene&       scene();

    image::Float4 const& target() const;

    void render(Exporters& exporters);

    void render(uint32_t frame);

    void start_frame(uint32_t frame);

    void render(uint32_t frame, uint32_t iteration);

    void postprocess();

    void export_frame(uint32_t frame, Exporters& exporters) const;

  private:
    void render_frame_backward(uint32_t frame);

    void render_frame_backward(uint32_t frame, uint32_t iteration);

    void render_frame_forward(uint32_t frame);

    void render_frame_forward(uint32_t frame, uint32_t iteration);

    void bake_photons(uint32_t frame);

    thread::Pool& threads_;

    Scene* scene_;

    take::View* view_;

    Camera_worker* workers_;

    uint32_t frame_;
    uint32_t frame_view_;
    uint32_t frame_iteration_;

    Tile_queue tiles_;

    Range_queue ranges_;

    image::Float4 target_;

    integrator::particle::photon::Map photon_map_;

    integrator::particle::Importance_cache particle_importance_;

    struct Photon_info {
        uint32_t num_paths;
    };

    Photon_info* photon_infos_;

    progress::Sink& progressor_;
};

}  // namespace rendering

#endif
