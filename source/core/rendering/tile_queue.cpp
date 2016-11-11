#include "tile_queue.hpp"
#include "base/math/vector.inl"

namespace rendering {

Tile_queue::Tile_queue(int2 resolution, int2 tile_dimensions, int32_t filter_radius) :
	tiles_(static_cast<size_t>(std::ceil(static_cast<float>(resolution.x) /
										 static_cast<float>(tile_dimensions.x)))
		 * static_cast<size_t>(std::ceil(static_cast<float>(resolution.y) /
										 static_cast<float>(tile_dimensions.y)))),
	current_produce_(0), current_consume_(0) {
	int2 current_pixel(0, 0);
	for (;;) {
		int2 start = current_pixel;
		int2 end   = math::min(current_pixel + tile_dimensions, resolution);

		if (0 == start.y) {
			start.y -= filter_radius;
		}

		if (resolution.y == end.y) {
			end.y += filter_radius;
		}

		if (0 == start.x) {
			start.x -= filter_radius;
		}

		if (resolution.x == end.x) {
			end.x += filter_radius;
		}

		push(math::Recti{start, end});

		current_pixel.x += tile_dimensions.x;

		if (current_pixel.x >= resolution.x) {
			current_pixel.x = 0;
			current_pixel.y += tile_dimensions.y;
		}

		if (current_pixel.y >= resolution.y) {
			break;
		}
	}
}

uint32_t Tile_queue::size() const {
	return static_cast<uint32_t>(tiles_.size());
}

void Tile_queue::restart() {
	current_produce_ = tiles_.size();
	current_consume_ = 0;
}

bool Tile_queue::pop(math::Recti& tile) {
	std::lock_guard<std::mutex> lock(mutex_);

	if (current_consume_ < tiles_.size()) {
		tile = tiles_[current_consume_++];
		return true;
	}

	return false;
}

void Tile_queue::push(const math::Recti& tile) {
	tiles_[current_produce_++] = tile;
}

}
