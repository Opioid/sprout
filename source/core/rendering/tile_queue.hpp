#pragma once

#include "base/math/vector.hpp"
#include "base/math/rectangle.hpp"
#include <condition_variable>
#include <vector>

namespace rendering {

class Tile_queue {
public:

	Tile_queue(math::int2 resolution, math::int2 tile_dimensions, int32_t filter_radius);

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

}
