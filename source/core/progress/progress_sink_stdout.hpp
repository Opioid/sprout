#pragma once

#include "progress_sink.hpp"

namespace progress {

class Stdout : public Sink {
public:

	virtual void start(size_t resolution) final override;
	virtual void end() final override;
	virtual void tick() final override;

private:

	size_t resolution_;
	size_t progress_;

	float step_;
	float threshold_;
};

}
