#pragma once

#include "progress_sink.hpp"

namespace progress {

class Null : public Sink {
public:

	virtual void start(uint32_t resolution) override final;
	virtual void end() override final;
	virtual void tick() override final;
};

}

