#pragma once

#include "sink.hpp"

namespace progress {

class Std_out : public Sink {
public:

	virtual void start(uint32_t resolution);
	virtual void end();
	virtual void tick();

private:

	uint32_t resolution_;
	uint32_t progress_;

	float step_;
	float threshold_;
};

}
