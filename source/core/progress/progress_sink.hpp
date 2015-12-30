#pragma once

#include <cstdint>

namespace progress {

class Sink {
public:

	virtual ~Sink() {}

	virtual void start(uint32_t resolution) = 0;
	virtual void end() = 0;
	virtual void tick() = 0;
};

}
