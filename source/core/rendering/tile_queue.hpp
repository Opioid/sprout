#ifndef SU_CORE_RENDERING_TILE_QUEUE_HPP
#define SU_CORE_RENDERING_TILE_QUEUE_HPP

#include "base/math/vector2.hpp"
#include "base/math/vector4.hpp"

#include <atomic>

namespace rendering {

class Tile_queue {
  public:
    ~Tile_queue();

    void init(int4_p crop, int32_t tile_dimensions, int32_t filter_radius);

    uint32_t size() const;

    void restart();

    bool pop(int4& tile);

  private:
    int4 crop_;

    int32_t tile_dimensions_;

    int32_t filter_radius_;

    int32_t tiles_per_row_;

    int32_t num_tiles_;

    std::atomic<int32_t> current_consume_;
};

class Range_queue {
  public:
    ~Range_queue();

    void init(uint64_t total0, uint64_t total1, uint32_t range_size);

    uint64_t total() const;

    uint32_t size() const;

    void restart(uint32_t segment);

    bool pop(ulong2& range);

  private:
    uint64_t total0_;
    uint64_t total1_;

    uint32_t range_size_;

    uint32_t num_ranges0_;
    uint32_t num_ranges1_;

    uint32_t current_segment_;

    std::atomic<uint32_t> current_consume_;
};

}  // namespace rendering

#endif
