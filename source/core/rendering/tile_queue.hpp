#pragma once

#include "base/math/vector.hpp"
#include "base/math/rectangle.hpp"
#include <condition_variable>
#include <vector>

namespace rendering {

class Tile_queue {

public:

	Tile_queue(int2 resolution, int2 tile_dimensions, int32_t filter_radius);

	uint32_t size() const;

	void restart();

	bool pop(math::Recti& tile);

private:

	void push(const math::Recti& tile);

	std::vector<math::Recti> tiles_;
	size_t current_produce_;
	size_t current_consume_;

	std::mutex mutex_;
};

class Tile_queue_lockfree {

public:

	Tile_queue_lockfree(int2 resolution, int2 tile_dimensions,
						int32_t filter_radius, uint32_t num_workers);

	~Tile_queue_lockfree();

	uint32_t size() const;

	void restart();

	bool pop(uint32_t id, math::Recti& tile);

private:

	void push(uint32_t id, const math::Recti& tile);

	struct Bin {
		std::vector<math::Recti> tiles;
		size_t current_consume;
	};

	uint32_t num_bins_;
	Bin* bins_;
};


}
