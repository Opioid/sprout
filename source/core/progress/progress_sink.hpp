#ifndef SU_PROGRESS_SINK_HPP
#define SU_PROGRESS_SINK_HPP

#include <cstdint>

namespace progress {

class Sink {
  public:
    virtual ~Sink() = default;

    virtual void start(uint32_t resolution) = 0;

    virtual void tick() = 0;
};

}  // namespace progress

#endif
