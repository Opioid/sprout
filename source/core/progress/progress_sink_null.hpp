#ifndef SU_PROGRESS_SINK_NULL_HPP
#define SU_PROGRESS_SINK_NULL_HPP

#include "progress_sink.hpp"

namespace progress {

class Null : public Sink {
  public:
    void start(uint32_t /*resolution*/) final {}

    void tick() final {}
};

}  // namespace progress

#endif
