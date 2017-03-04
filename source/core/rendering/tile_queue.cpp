#include "tile_queue.hpp"
#include "base/math/vector.inl"
#include "base/memory/align.inl"

namespace rendering {

Tile_queue::Tile_queue(int2 resolution, int2 tile_dimensions, int32_t filter_radius) :
	num_tiles_(static_cast<uint32_t>(std::ceil(static_cast<float>(resolution.v[0]) /
			   static_cast<float>(tile_dimensions.v[0]))) *
			   static_cast<uint32_t>(std::ceil(static_cast<float>(resolution.v[1]) /
			   static_cast<float>(tile_dimensions.v[1])))),
	tiles_(memory::allocate_aligned<int4>(num_tiles_)),
	current_consume_(num_tiles_) {
	int2 current_pixel(0, 0);
	for (;;) {
		int2 start = current_pixel;
		int2 end   = math::min(current_pixel + tile_dimensions, resolution);

		if (0 == start.v[1]) {
			start.v[1] -= filter_radius;
		}

		if (resolution.v[1] == end.v[1]) {
			end.v[1] += filter_radius;
		}

		if (0 == start.v[0]) {
			start.v[0] -= filter_radius;
		}

		if (resolution.v[0] == end.v[0]) {
			end.v[0] += filter_radius;
		}

		push(int4(start, end - int2(1)));

		current_pixel.v[0] += tile_dimensions.v[0];

		if (current_pixel.v[0] >= resolution.v[0]) {
			current_pixel.v[0] = 0;
			current_pixel.v[1] += tile_dimensions.v[1];
		}

		if (current_pixel.v[1] >= resolution.v[1]) {
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
	std::lock_guard<std::mutex> lock(mutex_);

	if (current_consume_ < num_tiles_) {
		tile = tiles_[current_consume_++];
		return true;
	}

	return false;
}

void Tile_queue::push(const int4& tile) {
	uint32_t current = num_tiles_ - current_consume_--;

	tiles_[current] = tile;
}

}
