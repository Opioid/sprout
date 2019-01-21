#ifndef SU_CORE_RENDERING_DRIVER_FINALFRAME_HPP
#define SU_CORE_RENDERING_DRIVER_FINALFRAME_HPP

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
    Driver_finalframe(take::Take& take, Scene& scene, thread::Pool& thread_pool,
                      uint32_t max_sample_size) noexcept;

    using Exporters = std::vector<exporting::Sink*>;

    void render(Exporters& exporters, progress::Sink& progressor) noexcept;

  private:
    void render_frame(uint32_t frame, progress::Sink& progressor) noexcept;

    void bake_photons(uint32_t frame) noexcept;

    bool photons_baked_;
};

}  // namespace rendering

#endif
