#pragma once

#include "progress_sink.hpp"

namespace progress {

class Stdout : public Sink {
public:

	Stdout();

	virtual void start(uint32_t resolution) override final;
	virtual void end() override final;
	virtual void tick() override final;

private:

	uint32_t resolution_;
	uint32_t progress_;

	float threshold_;

	const float step_;
};

}
