#pragma once

#include "progress_sink.hpp"

namespace progress {

class Stdout : public Sink {
public:

	Stdout();

	virtual void start(size_t resolution) final override;
	virtual void end() final override;
	virtual void tick() final override;

private:

	size_t resolution_;
	size_t progress_;

	float threshold_;

	const float step_;
};

}
