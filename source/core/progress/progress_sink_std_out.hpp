#ifndef SU_PROGRESS_SINK_STD_OUT_HPP
#define SU_PROGRESS_SINK_STD_OUT_HPP

#include "progress_sink.hpp"

#include <iostream>

namespace progress {

class Std_out : public Sink {
  public:
    void start(uint32_t resolution) noexcept override final {
        resolution_ = resolution;
        progress_   = 0;
        threshold_  = Step;
    }

    void tick() noexcept override final {
        if (progress_ >= resolution_) {
            return;
        }

        ++progress_;

        if (float const p = float(progress_) / float(resolution_) * 100.f; p >= threshold_) {
            threshold_ += Step;

            std::cout << uint32_t(p) << "%\r" << std::flush;
        }
    }

  private:
    uint32_t resolution_;
    uint32_t progress_;

    float threshold_;

    static float constexpr Step = 1.f;
};

}  // namespace progress

#endif
