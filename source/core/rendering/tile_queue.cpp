#include "tile_queue.hpp"
#include "base/math/vector4.inl"

namespace rendering {

Tile_queue::~Tile_queue() = default;

void Tile_queue::init(int4 const& crop, int32_t tile_dimensions, int32_t filter_radius) {
    crop_            = crop;
    tile_dimensions_ = tile_dimensions;
    filter_radius_   = filter_radius;

    int2 const dimensions = crop.zw() - crop.xy();

    int32_t const tiles_per_row = int32_t(std::ceil(float(dimensions[0]) / float(tile_dimensions)));

    tiles_per_row_ = tiles_per_row;

    num_tiles_ = tiles_per_row * int32_t(std::ceil(float(dimensions[1]) / float(tile_dimensions)));

    current_consume_ = 0;
}

uint32_t Tile_queue::size() const {
    return uint32_t(num_tiles_);
}

void Tile_queue::restart() {
    current_consume_ = 0;
}

bool Tile_queue::pop(int4& tile) {
    int32_t const current = current_consume_.fetch_add(1, std::memory_order_relaxed);

    if (current >= num_tiles_) {
        return false;
    }

    int4 const crop = crop_;

    int32_t const tile_dimensions = tile_dimensions_;

    int32_t const filter_radius = filter_radius_;

    int2 start;
    start[1] = current / tiles_per_row_;
    start[0] = current - start[1] * tiles_per_row_;

    start *= tile_dimensions;
    start += crop.xy();

    int2 end = min(start + tile_dimensions, crop.zw());

    if (crop[1] == start[1]) {
        start[1] -= filter_radius;
    }

    if (crop[3] == end[1]) {
        end[1] += filter_radius;
    }

    if (crop[0] == start[0]) {
        start[0] -= filter_radius;
    }

    if (crop[2] == end[0]) {
        end[0] += filter_radius;
    }

    tile = int4(start, end - 1);

    return true;
}

Range_queue::~Range_queue() = default;

void Range_queue::init(uint64_t total0, uint64_t total1, uint32_t range_size) {
    total0_ = total0;
    total1_ = total1;

    range_size_ = range_size;

    num_ranges0_ = uint32_t(std::ceil(float(total0) / float(range_size)));
    num_ranges1_ = uint32_t(std::ceil(float(total1) / float(range_size)));
}

uint64_t Range_queue::total() const {
    return total0_ + total1_;
}

uint32_t Range_queue::size() const {
    return num_ranges0_ + num_ranges1_;
}

void Range_queue::restart(uint32_t segment) {
    current_segment_ = segment;

    current_consume_ = 0;
}

bool Range_queue::pop(ulong2& range) {
    uint32_t const current = current_consume_.fetch_add(1, std::memory_order_relaxed);

    uint64_t const start = uint64_t(current) * uint64_t(range_size_) +
                           (0 == current_segment_ ? 0 : total0_);

    uint32_t const num_ranges = (0 == current_segment_ ? num_ranges0_ : num_ranges1_);

    if (current < num_ranges - 1) {
        range = ulong2(start, start + range_size_);
        return true;
    }

    if (current < num_ranges) {
        range = ulong2(start, 0 == current_segment_ ? total0_ : total1_);
        return true;
    }

    return false;
}

uint32_t Range_queue::index(ulong2 const& range, uint32_t segment) const {
    return uint32_t(range[0] / uint64_t(range_size_)) + segment * num_ranges0_;
}

}  // namespace rendering
