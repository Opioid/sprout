#ifndef SU_PROGRESS_SINK_STD_OUT_HPP
#define SU_PROGRESS_SINK_STD_OUT_HPP

#include "progress_sink.hpp"

namespace progress {

class Std_out : public Sink {
  public:
    Std_out();

    void start(uint32_t resolution) override final;
    void end() override final;
    void tick() override final;

  private:
    uint32_t resolution_;
    uint32_t progress_;

    float threshold_;

    float const step_;
};

}  // namespace progress

#endif
