#include "progress_sink_std_out.hpp"
#include <iostream>

namespace progress {

Std_out::Std_out() : step_(1.f) {}

void Std_out::start(uint32_t resolution) {
    resolution_ = resolution;
    progress_   = 0;
    threshold_  = step_;
}

void Std_out::end() {}

void Std_out::tick() {
    if (progress_ >= resolution_) {
        return;
    }

    ++progress_;

    if (float const p = float(progress_) / float(resolution_) * 100.f; p >= threshold_) {
        threshold_ += step_;

        std::cout << uint32_t(p) << "%\r" << std::flush;
    }
}

}  // namespace progress
