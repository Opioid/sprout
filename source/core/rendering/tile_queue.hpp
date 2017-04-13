#pragma once

#include "base/math/vector4.hpp"
//#include <condition_variable>
#include <atomic>

namespace rendering {

class Tile_queue {

public:

	Tile_queue(int2 resolution, int2 tile_dimensions, int32_t filter_radius);
	~Tile_queue();

	uint32_t size() const;

	void restart();

	bool pop(int4& tile);

private:

	void push(const int4& tile);

	uint32_t num_tiles_;
	int4* tiles_;

//	uint32_t current_consume_;

	std::atomic_uint32_t current_consume_;

//	std::mutex mutex_;
};

}
