#include "tile_queue.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"

namespace rendering {

Tile_queue::Tile_queue(int2 resolution, int2 tile_dimensions, int32_t filter_radius) noexcept
    : tile_dimensions_(tile_dimensions),
      tiles_per_row_(static_cast<int32_t>(
          std::ceil(static_cast<float>(resolution[0]) / static_cast<float>(tile_dimensions[0])))),
      num_tiles_(uint32_t(std::ceil(static_cast<float>(resolution[0]) /
                                    static_cast<float>(tile_dimensions[0]))) *
                 uint32_t(std::ceil(static_cast<float>(resolution[1]) /
                                    static_cast<float>(tile_dimensions[1])))),
      tiles_(memory::allocate_aligned<int4>(num_tiles_)),
      current_consume_(num_tiles_) {
    int2 current_pixel(0, 0);
    for (;;) {
        int2 start = current_pixel;
        int2 end   = min(current_pixel + tile_dimensions, resolution);

        if (0 == start[1]) {
            start[1] -= filter_radius;
        }

        if (resolution[1] == end[1]) {
            end[1] += filter_radius;
        }

        if (0 == start[0]) {
            start[0] -= filter_radius;
        }

        if (resolution[0] == end[0]) {
            end[0] += filter_radius;
        }

        push(int4(start, end - int2(1)));

        current_pixel[0] += tile_dimensions[0];

        if (current_pixel[0] >= resolution[0]) {
            current_pixel[0] = 0;
            current_pixel[1] += tile_dimensions[1];
        }

        if (current_pixel[1] >= resolution[1]) {
            break;
        }
    }
}

Tile_queue::~Tile_queue() noexcept {
    memory::free_aligned(tiles_);
}

uint32_t Tile_queue::size() const noexcept {
    return num_tiles_;
}

void Tile_queue::restart() noexcept {
    current_consume_ = 0;
}

bool Tile_queue::pop(int4& tile) noexcept {
    // uint32_t const current = current_consume_++;
    uint32_t const current = current_consume_.fetch_add(1, std::memory_order_relaxed);

    if (current < num_tiles_) {
        tile = tiles_[current];
        return true;
    }

    return false;
}

uint32_t Tile_queue::index(int4 const& tile) const noexcept {
    int32_t const x = std::max(tile[0], 0) / tile_dimensions_[0];
    int32_t const y = std::max(tile[1], 0) / tile_dimensions_[1];

    return uint32_t(y * tiles_per_row_ + x);
}

void Tile_queue::push(int4 const& tile) noexcept {
    uint32_t const current = num_tiles_ - current_consume_--;

    tiles_[current] = tile;
}

Range_queue::Range_queue(uint64_t total0, uint64_t total1, uint32_t range_size) noexcept
    : total0_(total0),
      total1_(total1),
      range_size_(range_size),
      num_ranges0_(
          uint32_t(std::ceil(static_cast<float>(total0) / static_cast<float>(range_size)))),
      num_ranges1_(uint32_t(std::ceil(static_cast<float>(total1) / static_cast<float>(range_size))))

{}

Range_queue::~Range_queue() noexcept {}

uint32_t Range_queue::size() const noexcept {
    return num_ranges0_ + num_ranges1_;
}

void Range_queue::restart() noexcept {
    current_consume_ = 0;
}

bool Range_queue::pop(uint32_t iteration, ulong2& range) noexcept {
    uint32_t const current = current_consume_.fetch_add(1, std::memory_order_relaxed);

    uint64_t const start = uint64_t(current) * uint64_t(range_size_);

    uint32_t const num_ranges = 0 == iteration ? num_ranges0_ : num_ranges1_;

    if (current < num_ranges - 1) {
        range = ulong2(start, start + range_size_);
        return true;
    }

    if (current < num_ranges) {
        range = ulong2(start, 0 == iteration ? total0_ : total1_);
        return true;
    }

    return false;
}

uint32_t Range_queue::index(ulong2 const& range, uint32_t iteration) const noexcept {
    return uint32_t(range[0] / uint64_t(range_size_)) + iteration * num_ranges0_;
}

}  // namespace rendering
