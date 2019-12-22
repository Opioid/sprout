#ifndef SU_CORE_RENDERING_TILE_QUEUE_HPP
#define SU_CORE_RENDERING_TILE_QUEUE_HPP

#include "base/math/vector2.hpp"
#include "base/math/vector4.hpp"

#include <atomic>

namespace rendering {

class Tile_queue {
  public:
    Tile_queue(int2 resolution, int32_t tile_dimensions, int32_t filter_radius) noexcept;

    ~Tile_queue() noexcept;

    uint32_t size() const noexcept;

    void restart() noexcept;

    bool pop(int4& tile) noexcept;

    uint32_t index(int4 const& tile) const noexcept;

  private:
	int2 const resolution_;

    int32_t const tile_dimensions_;

	int32_t const filter_radius_;

    int32_t const tiles_per_row_;

    uint32_t const num_tiles_;

    std::atomic<uint32_t> current_consume_;
};

class Range_queue {
  public:
    Range_queue(uint64_t total0, uint64_t total1, uint32_t range_size) noexcept;

    ~Range_queue() noexcept;

    uint32_t size() const noexcept;

    void restart() noexcept;

    bool pop(uint32_t iteration, ulong2& range) noexcept;

    uint32_t index(ulong2 const& range, uint32_t iteration) const noexcept;

  private:
    uint64_t const total0_;
    uint64_t const total1_;

    uint32_t const range_size_;

    uint32_t const num_ranges0_;
    uint32_t const num_ranges1_;

    std::atomic<uint32_t> current_consume_;
};

}  // namespace rendering

#endif
