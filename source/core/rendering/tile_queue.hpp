#ifndef SU_CORE_RENDERING_TILE_QUEUE_HPP
#define SU_CORE_RENDERING_TILE_QUEUE_HPP

#include <atomic>
#include "base/math/vector4.hpp"

namespace rendering {

class Tile_queue {
  public:
    Tile_queue(int2 resolution, int2 tile_dimensions, int32_t filter_radius) noexcept;

    ~Tile_queue() noexcept;

    uint32_t size() const noexcept;

    void restart() noexcept;

    bool pop(int4& tile) noexcept;

  private:
    void push(int4 const& tile) noexcept;

    uint32_t num_tiles_;
    int4*    tiles_;

    std::atomic<uint32_t> current_consume_;
};

}  // namespace rendering

#endif
