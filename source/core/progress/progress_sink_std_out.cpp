#include "progress_sink_std_out.hpp"
#include <iostream>

namespace progress {

Std_out::Std_out() : step_(1.f) {}

void Std_out::start(uint32_t resolution) {
	resolution_ = resolution;
	progress_ = 0;
	threshold_ = step_;
}

void Std_out::end() {
	std::cout << std::endl;
}

void Std_out::tick() {
	if (progress_ >= resolution_) {
		return;
	}

	++progress_;

	const float p = static_cast<float>(progress_) / static_cast<float>(resolution_) * 100.f;
	if (p >= threshold_) {
		threshold_ += step_;

		std::cout << static_cast<uint32_t>(p) << "%\r" << std::flush;
	}
}

}
