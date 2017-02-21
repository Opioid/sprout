#pragma once

#include "base/math/vector.hpp"
#include <condition_variable>
#include <vector>

namespace rendering {

class Tile_queue {

public:

	Tile_queue(int2 resolution, int2 tile_dimensions, int32_t filter_radius);

	uint32_t size() const;

	void restart();

	bool pop(int4& tile);

private:

	void push(const int4& tile);

	std::vector<int4> tiles_;
	size_t current_consume_;

	std::mutex mutex_;
};

}
