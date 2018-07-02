#ifndef SU_CORE_RENDERING_DRIVER_FINALFRAME_HPP
#define SU_CORE_RENDERING_DRIVER_FINALFRAME_HPP

#include <memory>
#include <vector>
#include "rendering_driver.hpp"

namespace scene::camera {
class Camera;
}

namespace exporting {
class Sink;
}

namespace progress {
class Sink;
}

namespace rendering {

class Driver_finalframe : public Driver {
  public:
    Driver_finalframe(take::Take& take, scene::Scene& scene, thread::Pool& thread_pool,
                      uint32_t max_sample_size);

    using Exporters = std::vector<std::unique_ptr<exporting::Sink>>;

    void render(Exporters& exporters, progress::Sink& progressor);

  private:
    void render_subframe(float normalized_tick_offset, float normalized_tick_slice,
                         float normalized_frame_slice, progress::Sink& progressor);

    void bake_photons(float normalized_tick_offset, float normalized_tick_slice);

    static uint32_t calculate_progress_range(scene::Scene const&          scene,
                                             const scene::camera::Camera& camera,
                                             uint32_t                     num_tiles,
                                             uint32_t num_samples_per_iteration);

    uint32_t current_sample_;

    bool photons_baked_;
};

}  // namespace rendering

#endif
