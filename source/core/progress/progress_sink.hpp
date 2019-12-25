#ifndef SU_PROGRESS_SINK_HPP
#define SU_PROGRESS_SINK_HPP

#include <cstdint>

namespace progress {

class Sink {
  public:
    virtual ~Sink() noexcept {}

    virtual void start(uint32_t resolution) noexcept = 0;

    virtual void tick() noexcept = 0;
};

}  // namespace progress

#endif
