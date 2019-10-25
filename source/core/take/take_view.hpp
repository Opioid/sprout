#ifndef SU_CORE_TAKE_VIEW_HPP
#define SU_CORE_TAKE_VIEW_HPP

#include "rendering/postprocessor/postprocessor_pipeline.hpp"

namespace scene::camera {
class Camera;
}

namespace take {

struct View {
    View() noexcept;

    ~View() noexcept;

    void clear() noexcept;

    void init(thread::Pool& threads) noexcept;

    scene::camera::Camera* camera = nullptr;

    uint32_t num_samples_per_pixel = 1;

    rendering::postprocessor::Pipeline pipeline;

    uint32_t start_frame = 0;
    uint32_t num_frames  = 1;
};

}  // namespace take

#endif
