#pragma once

#include <cstddef>

namespace progress {

class Sink {
public:

	virtual ~Sink() {}

	virtual void start(size_t resolution) = 0;
	virtual void end() = 0;
	virtual void tick() = 0;
};

}
