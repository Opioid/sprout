#pragma once

#include "rectangle.hpp"
#include "base/math/vector.hpp"
#include <condition_variable>
#include <vector>

namespace rendering {

class Tile_queue {
public:

	Tile_queue(math::uint2 resolution, math::uint2 tile_dimensions);

	size_t size() const;

	void restart();

	bool pop(Rectui& tile);

private:

	void push(const Rectui& tile);

	std::vector<Rectui> tiles_;
	size_t current_produce_;
	size_t current_consume_;

	std::mutex mutex_;
};

}
