#pragma once

#include "progress_sink.hpp"

namespace progress {

class Stdout : public Sink {
public:

	Stdout();

	virtual void start(uint32_t resolution) final override;
	virtual void end() final override;
	virtual void tick() final override;

private:

	uint32_t resolution_;
	uint32_t progress_;

	float threshold_;

	const float step_;
};

}
