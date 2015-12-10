#pragma once

#include "progress_sink.hpp"

namespace progress {

class Null : public Sink {
public:

	virtual void start(uint32_t resolution) final override;
	virtual void end() final override;
	virtual void tick() final override;
};

}

