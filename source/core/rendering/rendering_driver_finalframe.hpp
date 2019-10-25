#ifndef SU_CORE_RENDERING_DRIVER_FINALFRAME_HPP
#define SU_CORE_RENDERING_DRIVER_FINALFRAME_HPP

#include "base/memory/array.hpp"
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
                      uint32_t max_sample_size, progress::Sink& progressor) noexcept;

    using Exporters = memory::Array<exporting::Sink*>;

    void render(Exporters& exporters) noexcept;

  private:
    void render_frame_backward(uint32_t frame) noexcept;

    void render_frame_forward(uint32_t frame) noexcept;

    void bake_photons(uint32_t frame) noexcept;

    progress::Sink& progressor_;
};

}  // namespace rendering

#endif
