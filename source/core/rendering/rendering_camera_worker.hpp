#ifndef SU_CORE_RENDERING_CAMERA_WORKER_HPP
#define SU_CORE_RENDERING_CAMERA_WORKER_HPP

#include "rendering_worker.hpp"

namespace rendering {

class alignas(64) Camera_worker : public Worker {
  public:
    Camera_worker();

    void render(uint32_t frame, uint32_t view, uint32_t iteration, int4 const& tile,
                uint32_t num_samples);

    void particles(uint32_t frame, uint64_t offset, ulong2 const& range);
};

}  // namespace rendering

#endif
