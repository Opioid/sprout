#include "tile_queue.hpp"
#include "base/math/vector4.inl"

namespace rendering {

Tile_queue::~Tile_queue() noexcept {}

void Tile_queue::init(int2 resolution, int32_t tile_dimensions, int32_t filter_radius) noexcept {
    resolution_      = resolution;
    tile_dimensions_ = tile_dimensions;
    filter_radius_   = filter_radius;

    int32_t const tiles_per_row = int32_t(std::ceil(float(resolution[0]) / float(tile_dimensions)));

    tiles_per_row_ = tiles_per_row;

    num_tiles_ = tiles_per_row_ * int32_t(std::ceil(float(resolution[1]) / float(tile_dimensions)));

    current_consume_ = 0;
}

uint32_t Tile_queue::size() const noexcept {
    return uint32_t(num_tiles_);
}

void Tile_queue::restart() noexcept {
    current_consume_ = 0;
}

bool Tile_queue::pop(int4& tile) noexcept {
    // uint32_t const current = current_consume_++;
    int32_t const current = current_consume_.fetch_add(1, std::memory_order_relaxed);

    if (current < num_tiles_) {
        int2 const resolution = resolution_;

        int32_t const tile_dimensions = tile_dimensions_;

        int32_t const filter_radius = filter_radius_;

        int2 start;
        start[1] = current / tiles_per_row_;
        start[0] = current - start[1] * tiles_per_row_;

        start *= tile_dimensions;

        int2 end = min(start + tile_dimensions, resolution);

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

        tile = int4(start, end - 1);

        return true;
    }

    return false;
}

uint32_t Tile_queue::index(int4 const& tile) const noexcept {
    int32_t const x = std::max(tile[0], 0) / tile_dimensions_;
    int32_t const y = std::max(tile[1], 0) / tile_dimensions_;

    return uint32_t(y * tiles_per_row_ + x);
}

Range_queue::~Range_queue() noexcept {}

void Range_queue::init(uint64_t total0, uint64_t total1, uint32_t range_size) noexcept {
    total0_ = total0;
    total1_ = total1;

    range_size_ = range_size;

    num_ranges0_ = uint32_t(std::ceil(float(total0) / float(range_size)));
    num_ranges1_ = uint32_t(std::ceil(float(total1) / float(range_size)));
}

uint32_t Range_queue::size() const noexcept {
    return num_ranges0_ + num_ranges1_;
}

void Range_queue::restart() noexcept {
    current_consume_ = 0;
}

bool Range_queue::pop(uint32_t segment, ulong2& range) noexcept {
    uint32_t const current = current_consume_.fetch_add(1, std::memory_order_relaxed);

    uint64_t const start = uint64_t(current) * uint64_t(range_size_);

    uint32_t const num_ranges = 0 == segment ? num_ranges0_ : num_ranges1_;

    if (current < num_ranges - 1) {
        range = ulong2(start, start + range_size_);
        return true;
    }

    if (current < num_ranges) {
        range = ulong2(start, 0 == segment ? total0_ : total1_);
        return true;
    }

    return false;
}

uint32_t Range_queue::index(ulong2 const& range, uint32_t segment) const noexcept {
    uint64_t const range_size = uint64_t(range_size_);

    uint64_t const r = range[0] % range_size;

    if (0 == r) {
        return uint32_t(range[0] / range_size);
    }

    return uint32_t((range[0] + r) / range_size);
}

}  // namespace rendering
