#pragma once

#include "sink.hpp"

namespace progress {

class Std_out : public Sink {
public:

	virtual void start(size_t resolution);
	virtual void end();
	virtual void tick();

private:

	size_t resolution_;
	size_t progress_;

	float step_;
	float threshold_;
};

}
