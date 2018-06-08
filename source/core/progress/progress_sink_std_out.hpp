#pragma once

#include "progress_sink.hpp"

namespace progress {

class Std_out : public Sink {
  public:
    Std_out();

    virtual void start(uint32_t resolution) override final;
    virtual void end() override final;
    virtual void tick() override final;

  private:
    uint32_t resolution_;
    uint32_t progress_;

    float threshold_;

    float const step_;
};

}  // namespace progress
