#include "tile_queue.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"

namespace rendering {

Tile_queue::Tile_queue(int2 resolution, int2 tile_dimensions, int32_t filter_radius)
    : num_tiles_(static_cast<uint32_t>(std::ceil(static_cast<float>(resolution[0]) /
                                                 static_cast<float>(tile_dimensions[0]))) *
                 static_cast<uint32_t>(std::ceil(static_cast<float>(resolution[1]) /
                                                 static_cast<float>(tile_dimensions[1])))),
      tiles_(memory::allocate_aligned<int4>(num_tiles_)),
      current_consume_(num_tiles_) {
    int2 current_pixel(0, 0);
    for (;;) {
        int2 start = current_pixel;
        int2 end   = math::min(current_pixel + tile_dimensions, resolution);

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

Tile_queue::~Tile_queue() {
    memory::free_aligned(tiles_);
}

uint32_t Tile_queue::size() const {
    return num_tiles_;
}

void Tile_queue::restart() {
    current_consume_ = 0;
}

bool Tile_queue::pop(int4& tile) {
    // uint32_t const current = current_consume_++;
    uint32_t const current = current_consume_.fetch_add(1, std::memory_order_relaxed);

    if (current < num_tiles_) {
        tile = tiles_[current];
        return true;
    }

    return false;
}

void Tile_queue::push(int4 const& tile) {
    uint32_t const current = num_tiles_ - current_consume_--;

    tiles_[current] = tile;
}

}  // namespace rendering
