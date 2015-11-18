#include "progress_sink_stdout.hpp"
#include <iostream>

namespace progress {

Stdout::Stdout() : step_(1.f) {}

void Stdout::start(size_t resolution) {
	resolution_ = resolution;
	progress_ = 0;
	threshold_ = step_;
}

void Stdout::end() {
	std::cout << std::endl;
}

void Stdout::tick() {
	if (progress_ >= resolution_) {
		return;
	}

	++progress_;

	float p = static_cast<float>(progress_) / static_cast<float>(resolution_) * 100.f;

	if (p >= threshold_) {
		threshold_ += step_;

		std::cout << static_cast<size_t>(p) << "% \r" << std::flush;
	}
}

}
