#ifndef SU_CORE_RENDERING_CAMERA_WORKER_HPP
#define SU_CORE_RENDERING_CAMERA_WORKER_HPP

#include "rendering_worker.hpp"

namespace rendering {

class Tile_queue;
class Range_queue;

class alignas(64) Camera_worker : public Worker {
  public:
    Camera_worker(uint32_t max_sample_size, Tile_queue const& tiles, Range_queue const& ranges_);

    void render(uint32_t frame, uint32_t view, uint32_t iteration, int4 const& tile,
                uint32_t num_samples) noexcept;

    void particles(uint32_t frame, uint32_t view, uint32_t iteration, uint32_t segment,
                   ulong2 const& range) noexcept;

  private:
    Tile_queue const& tiles_;

    Range_queue const& ranges_;
};

}  // namespace rendering

#endif
