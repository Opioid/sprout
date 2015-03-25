#include "std_out.hpp"
#include <iostream>

namespace progress {

void Std_out::start(size_t resolution) {
	resolution_ = resolution;
	progress_ = 0;
	step_ = 10.f;
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

	float p = static_cast<float>(progress_) / static_cast<float>(resolution_) * 100.f;

	if (p >= threshold_) {
		threshold_ += step_;

		std::cout << static_cast<size_t>(p) << "%... " << std::flush;
	}
}

}
